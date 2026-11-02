#include "can/frame.hpp"
#include "can/virtual_can_bus.hpp"
#include "ecu/ecu.hpp"
#include "ecu/frame_factory.hpp"
#include "validation/frame_validator.hpp"

#include <iomanip>
#include <iostream>
#include <string>

namespace {

void print_event_log(const ecu_sim::SimulatedEcu& ecu) {
    std::cout << "\nEvent log\n";
    std::cout << "---------\n";
    for (const auto& event : ecu.event_log()) {
        std::cout << "[" << std::right << std::setw(5) << event.timestamp_ms << " ms] "
                  << std::left << std::setw(20) << event.event_type
                  << " " << event.details << '\n';
    }
}

int run_demo() {
    can_sim::VirtualCanBus bus;
    ecu_sim::SimulatedEcu ecu;
    bus.subscribe([&ecu](const can_sim::CanFrame& frame) { ecu.receive(frame); });

    std::cout << "ECU / CAN Validation Bench — software-only demonstration\n";
    std::cout << "========================================================\n";

    // Nominal simulated traffic.
    bus.transmit(ecu_sim::FrameFactory::vehicle_speed(87.5, 100));
    bus.transmit(ecu_sim::FrameFactory::coolant_temperature(92.0, 110));
    bus.transmit(ecu_sim::FrameFactory::accelerator_pedal(32.0, 120));

    // Unknown message: rejected and logged, but not classified as a safety-relevant signal failure.
    can_sim::CanFrame unknown;
    unknown.id = 0x555;
    unknown.dlc = 1;
    unknown.data[0] = 0x2A;
    unknown.timestamp_ms = 130;
    bus.transmit(unknown);

    // Safety-relevant malformed pedal signal: the simulated ECU enters SAFE_STATE.
    can_sim::CanFrame malformed_pedal;
    malformed_pedal.id = validation::FrameValidator::kPedalCanId;
    malformed_pedal.dlc = 1;
    malformed_pedal.data[0] = 150;
    malformed_pedal.timestamp_ms = 140;
    bus.transmit(malformed_pedal);

    // The following valid frame is intentionally ignored after SAFE_STATE.
    bus.transmit(ecu_sim::FrameFactory::vehicle_speed(10.0, 150));
    const std::size_t dispatched = bus.dispatch();

    const auto& stats = ecu.statistics();
    std::cout << "\nSimulation summary\n";
    std::cout << "------------------\n";
    std::cout << "Frames transmitted : " << bus.transmitted_frames() << '\n';
    std::cout << "Frames dispatched  : " << dispatched << '\n';
    std::cout << "Accepted frames    : " << stats.accepted_frames << '\n';
    std::cout << "Rejected frames    : " << stats.rejected_frames << '\n';
    std::cout << "Ignored in safe state: " << stats.ignored_after_safe_state << '\n';
    std::cout << "ECU state          : " << ecu_sim::to_string(ecu.state()) << '\n';

    const auto& telemetry = ecu.telemetry();
    std::cout << std::fixed << std::setprecision(1);
    if (telemetry.vehicle_speed_kmh) {
        std::cout << "Last accepted speed: " << *telemetry.vehicle_speed_kmh << " km/h\n";
    }
    if (telemetry.coolant_temperature_c) {
        std::cout << "Coolant temperature: " << *telemetry.coolant_temperature_c << " C\n";
    }
    if (telemetry.accelerator_pedal_percent) {
        std::cout << "Pedal position     : " << *telemetry.accelerator_pedal_percent << " %\n";
    }
    print_event_log(ecu);

    std::cout << "\nNote: all frames and ECU behaviour above are simulated in-process.\n";
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc > 1) {
        const std::string argument{argv[1]};
        if (argument == "--help" || argument == "-h") {
            std::cout << "Usage: ecu_can_bench [--demo]\n"
                      << "Run the isolated software CAN/ECU demonstration.\n";
            return 0;
        }
        if (argument != "--demo") {
            std::cerr << "Unknown option: " << argument << "\nUse --help for usage.\n";
            return 2;
        }
    }
    return run_demo();
}
