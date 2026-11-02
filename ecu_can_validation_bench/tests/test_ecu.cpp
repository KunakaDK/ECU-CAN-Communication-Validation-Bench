#include "test_framework.hpp"

#include "can/virtual_can_bus.hpp"
#include "ecu/ecu.hpp"
#include "ecu/frame_factory.hpp"
#include "validation/frame_validator.hpp"

TEST_CASE(ecu_updates_telemetry_from_valid_frames) {
    ecu_sim::SimulatedEcu ecu;
    ecu.receive(ecu_sim::FrameFactory::vehicle_speed(87.5, 10));
    ecu.receive(ecu_sim::FrameFactory::coolant_temperature(92.0, 20));
    ecu.receive(ecu_sim::FrameFactory::accelerator_pedal(32.0, 30));
    REQUIRE(ecu.state() == ecu_sim::EcuState::Operational);
    REQUIRE(ecu.statistics().accepted_frames == 3U);
    REQUIRE(ecu.telemetry().vehicle_speed_kmh.has_value());
    REQUIRE(ecu.telemetry().coolant_temperature_c.has_value());
    REQUIRE(ecu.telemetry().accelerator_pedal_percent.has_value());
    REQUIRE(*ecu.telemetry().vehicle_speed_kmh == 87.5);
    REQUIRE(*ecu.telemetry().coolant_temperature_c == 92.0);
    REQUIRE(*ecu.telemetry().accelerator_pedal_percent == 32.0);
}

TEST_CASE(unknown_message_is_rejected_without_safe_state_transition) {
    ecu_sim::SimulatedEcu ecu;
    can_sim::CanFrame unknown;
    unknown.id = 0x555;
    unknown.dlc = 1;
    ecu.receive(unknown);
    REQUIRE(ecu.statistics().rejected_frames == 1U);
    REQUIRE(ecu.state() == ecu_sim::EcuState::Operational);
    REQUIRE(ecu.last_diagnostic() == validation::DiagnosticCode::UnknownCanId);
}

TEST_CASE(safety_relevant_signal_failure_enters_safe_state) {
    ecu_sim::SimulatedEcu ecu;
    can_sim::CanFrame invalid_pedal;
    invalid_pedal.id = validation::FrameValidator::kPedalCanId;
    invalid_pedal.dlc = 1;
    invalid_pedal.data[0] = 130;
    invalid_pedal.timestamp_ms = 100;
    ecu.receive(invalid_pedal);
    REQUIRE(ecu.state() == ecu_sim::EcuState::SafeState);
    REQUIRE(ecu.statistics().rejected_frames == 1U);
    REQUIRE(ecu.event_log().size() == 2U);
    REQUIRE(ecu.event_log()[1].event_type == "SAFE_STATE_ENTERED");
}

TEST_CASE(ecu_ignores_subsequent_frames_after_entering_safe_state) {
    ecu_sim::SimulatedEcu ecu;
    can_sim::CanFrame invalid_pedal;
    invalid_pedal.id = validation::FrameValidator::kPedalCanId;
    invalid_pedal.dlc = 1;
    invalid_pedal.data[0] = 130;
    ecu.receive(invalid_pedal);

    ecu.receive(ecu_sim::FrameFactory::vehicle_speed(55.0, 200));
    REQUIRE(ecu.state() == ecu_sim::EcuState::SafeState);
    REQUIRE(ecu.statistics().ignored_after_safe_state == 1U);
    REQUIRE(ecu.statistics().accepted_frames == 0U);
    REQUIRE(!ecu.telemetry().vehicle_speed_kmh.has_value());
}

TEST_CASE(bus_to_ecu_end_to_end_demo_processes_nominal_and_fault_frames) {
    can_sim::VirtualCanBus bus;
    ecu_sim::SimulatedEcu ecu;
    bus.subscribe([&ecu](const can_sim::CanFrame& frame) { ecu.receive(frame); });

    bus.transmit(ecu_sim::FrameFactory::vehicle_speed(45.0, 1));
    bus.transmit(ecu_sim::FrameFactory::coolant_temperature(85.0, 2));
    can_sim::CanFrame malformed;
    malformed.id = validation::FrameValidator::kPedalCanId;
    malformed.dlc = 2;
    malformed.data[0] = 20;
    malformed.timestamp_ms = 3;
    bus.transmit(malformed);

    REQUIRE(bus.dispatch() == 3U);
    REQUIRE(ecu.statistics().accepted_frames == 2U);
    REQUIRE(ecu.statistics().rejected_frames == 1U);
    REQUIRE(ecu.state() == ecu_sim::EcuState::SafeState);
}

TEST_CASE(rejected_frame_does_not_overwrite_last_valid_telemetry) {
    ecu_sim::SimulatedEcu ecu;
    ecu.receive(ecu_sim::FrameFactory::vehicle_speed(63.0, 1));

    can_sim::CanFrame invalid_speed;
    invalid_speed.id = validation::FrameValidator::kSpeedCanId;
    invalid_speed.dlc = 2;
    invalid_speed.data[0] = 0x09;
    invalid_speed.data[1] = 0xC5; // 250.1 km/h, outside the test profile
    invalid_speed.timestamp_ms = 2;
    ecu.receive(invalid_speed);

    REQUIRE(ecu.state() == ecu_sim::EcuState::SafeState);
    REQUIRE(ecu.telemetry().vehicle_speed_kmh.has_value());
    REQUIRE(*ecu.telemetry().vehicle_speed_kmh == 63.0);
}
