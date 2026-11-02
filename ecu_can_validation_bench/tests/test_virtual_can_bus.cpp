#include "test_framework.hpp"

#include "can/virtual_can_bus.hpp"
#include "ecu/frame_factory.hpp"

#include <vector>

TEST_CASE(bus_queues_and_dispatches_frames_in_fifo_order) {
    can_sim::VirtualCanBus bus;
    std::vector<std::uint64_t> timestamps;
    bus.subscribe([&timestamps](const can_sim::CanFrame& frame) {
        timestamps.push_back(frame.timestamp_ms);
    });
    bus.transmit(ecu_sim::FrameFactory::vehicle_speed(10.0, 1));
    bus.transmit(ecu_sim::FrameFactory::vehicle_speed(20.0, 2));
    REQUIRE(bus.pending_frames() == 2U);
    REQUIRE(bus.dispatch() == 2U);
    REQUIRE(timestamps.size() == 2U);
    REQUIRE(timestamps[0] == 1U);
    REQUIRE(timestamps[1] == 2U);
    REQUIRE(bus.pending_frames() == 0U);
}

TEST_CASE(bus_delivers_each_frame_to_each_subscriber) {
    can_sim::VirtualCanBus bus;
    std::size_t first_count = 0;
    std::size_t second_count = 0;
    bus.subscribe([&first_count](const can_sim::CanFrame&) { ++first_count; });
    bus.subscribe([&second_count](const can_sim::CanFrame&) { ++second_count; });
    bus.transmit(ecu_sim::FrameFactory::vehicle_speed(10.0));
    bus.transmit(ecu_sim::FrameFactory::coolant_temperature(80.0));
    bus.dispatch();
    REQUIRE(first_count == 2U);
    REQUIRE(second_count == 2U);
    REQUIRE(bus.delivered_frames() == 4U);
}
