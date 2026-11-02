#include "test_framework.hpp"

#include "ecu/frame_factory.hpp"
#include "validation/frame_validator.hpp"

#include <stdexcept>

TEST_CASE(accepts_valid_vehicle_speed_frame) {
    validation::FrameValidator validator;
    const auto result = validator.validate(ecu_sim::FrameFactory::vehicle_speed(87.5));
    REQUIRE(result.accepted);
    REQUIRE(result.signal == validation::SignalType::VehicleSpeed);
    REQUIRE(result.physical_value == 87.5);
}

TEST_CASE(accepts_valid_coolant_temperature_frame) {
    validation::FrameValidator validator;
    const auto result = validator.validate(ecu_sim::FrameFactory::coolant_temperature(92.0));
    REQUIRE(result.accepted);
    REQUIRE(result.signal == validation::SignalType::CoolantTemperature);
    REQUIRE(result.physical_value == 92.0);
}

TEST_CASE(accepts_valid_accelerator_pedal_frame) {
    validation::FrameValidator validator;
    const auto result = validator.validate(ecu_sim::FrameFactory::accelerator_pedal(32.0));
    REQUIRE(result.accepted);
    REQUIRE(result.signal == validation::SignalType::AcceleratorPedal);
    REQUIRE(result.physical_value == 32.0);
}

TEST_CASE(rejects_standard_identifier_above_11_bits) {
    validation::FrameValidator validator;
    can_sim::CanFrame frame;
    frame.id = 0x800;
    frame.dlc = 1;
    const auto result = validator.validate(frame);
    REQUIRE(!result.accepted);
    REQUIRE(result.code == validation::DiagnosticCode::InvalidStandardId);
}

TEST_CASE(rejects_extended_identifier_in_this_profile) {
    validation::FrameValidator validator;
    auto frame = ecu_sim::FrameFactory::vehicle_speed(20.0);
    frame.extended_id = true;
    const auto result = validator.validate(frame);
    REQUIRE(!result.accepted);
    REQUIRE(result.code == validation::DiagnosticCode::UnsupportedExtendedId);
}

TEST_CASE(rejects_dlc_greater_than_classic_can_limit) {
    validation::FrameValidator validator;
    auto frame = ecu_sim::FrameFactory::accelerator_pedal(30.0);
    frame.dlc = 9;
    const auto result = validator.validate(frame);
    REQUIRE(!result.accepted);
    REQUIRE(result.code == validation::DiagnosticCode::DlcExceedsClassicCan);
    REQUIRE(result.safety_relevant);
}

TEST_CASE(rejects_unexpected_dlc_for_known_message) {
    validation::FrameValidator validator;
    auto frame = ecu_sim::FrameFactory::vehicle_speed(20.0);
    frame.dlc = 1;
    const auto result = validator.validate(frame);
    REQUIRE(!result.accepted);
    REQUIRE(result.code == validation::DiagnosticCode::UnexpectedDlc);
    REQUIRE(result.safety_relevant);
}

TEST_CASE(rejects_unknown_can_identifier) {
    validation::FrameValidator validator;
    can_sim::CanFrame frame;
    frame.id = 0x555;
    frame.dlc = 1;
    const auto result = validator.validate(frame);
    REQUIRE(!result.accepted);
    REQUIRE(result.code == validation::DiagnosticCode::UnknownCanId);
    REQUIRE(!result.safety_relevant);
}

TEST_CASE(rejects_vehicle_speed_outside_configured_range) {
    validation::FrameValidator validator;
    can_sim::CanFrame frame;
    frame.id = validation::FrameValidator::kSpeedCanId;
    frame.dlc = 2;
    frame.data[0] = 0x09;
    frame.data[1] = 0xC5; // 2501 * 0.1 = 250.1 km/h
    const auto result = validator.validate(frame);
    REQUIRE(!result.accepted);
    REQUIRE(result.code == validation::DiagnosticCode::SignalOutOfRange);
}

TEST_CASE(rejects_coolant_temperature_outside_configured_range) {
    validation::FrameValidator validator;
    can_sim::CanFrame frame;
    frame.id = validation::FrameValidator::kCoolantCanId;
    frame.dlc = 1;
    frame.data[0] = 191; // 191 - 40 = 151 C
    const auto result = validator.validate(frame);
    REQUIRE(!result.accepted);
    REQUIRE(result.code == validation::DiagnosticCode::SignalOutOfRange);
}

TEST_CASE(rejects_accelerator_value_above_100_percent) {
    validation::FrameValidator validator;
    can_sim::CanFrame frame;
    frame.id = validation::FrameValidator::kPedalCanId;
    frame.dlc = 1;
    frame.data[0] = 101;
    const auto result = validator.validate(frame);
    REQUIRE(!result.accepted);
    REQUIRE(result.code == validation::DiagnosticCode::SignalOutOfRange);
}

TEST_CASE(frame_factory_rejects_values_outside_signal_ranges) {
    bool speed_threw = false;
    bool temp_threw = false;
    bool pedal_threw = false;
    try { (void)ecu_sim::FrameFactory::vehicle_speed(251.0); }
    catch (const std::out_of_range&) { speed_threw = true; }
    try { (void)ecu_sim::FrameFactory::coolant_temperature(151.0); }
    catch (const std::out_of_range&) { temp_threw = true; }
    try { (void)ecu_sim::FrameFactory::accelerator_pedal(101.0); }
    catch (const std::out_of_range&) { pedal_threw = true; }
    REQUIRE(speed_threw);
    REQUIRE(temp_threw);
    REQUIRE(pedal_threw);
}
