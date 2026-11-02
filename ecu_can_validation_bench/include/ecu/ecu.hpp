#pragma once

#include "can/frame.hpp"
#include "validation/frame_validator.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace ecu_sim {

enum class EcuState {
    Operational,
    SafeState
};

struct EcuTelemetry {
    std::optional<double> vehicle_speed_kmh;
    std::optional<double> coolant_temperature_c;
    std::optional<double> accelerator_pedal_percent;
};

struct EcuStatistics {
    std::size_t accepted_frames{0};
    std::size_t rejected_frames{0};
    std::size_t ignored_after_safe_state{0};
};

struct EcuEvent {
    std::uint64_t timestamp_ms{0};
    std::string event_type;
    std::string details;
};

class SimulatedEcu {
public:
    void receive(const can_sim::CanFrame& frame);

    EcuState state() const noexcept;
    const EcuTelemetry& telemetry() const noexcept;
    const EcuStatistics& statistics() const noexcept;
    const std::vector<EcuEvent>& event_log() const noexcept;
    validation::DiagnosticCode last_diagnostic() const noexcept;

private:
    void enter_safe_state(const can_sim::CanFrame& frame,
                          const validation::ValidationResult& result);

    validation::FrameValidator validator_;
    EcuState state_{EcuState::Operational};
    EcuTelemetry telemetry_;
    EcuStatistics statistics_;
    std::vector<EcuEvent> event_log_;
    validation::DiagnosticCode last_diagnostic_{validation::DiagnosticCode::None};
};

std::string to_string(EcuState state);

} // namespace ecu_sim
