#include "can/virtual_can_bus.hpp"

#include <stdexcept>
#include <utility>

namespace can_sim {

void VirtualCanBus::subscribe(Receiver receiver) {
    if (!receiver) {
        throw std::invalid_argument("Virtual CAN receiver must be callable");
    }
    receivers_.push_back(std::move(receiver));
}

void VirtualCanBus::transmit(const CanFrame& frame) {
    queue_.push_back(frame);
    ++transmitted_frames_;
}

std::size_t VirtualCanBus::dispatch() {
    std::size_t dispatched = 0;
    while (!queue_.empty()) {
        const CanFrame frame = queue_.front();
        queue_.pop_front();
        for (const auto& receiver : receivers_) {
            receiver(frame);
            ++delivered_frames_;
        }
        ++dispatched;
    }
    return dispatched;
}

std::size_t VirtualCanBus::pending_frames() const noexcept {
    return queue_.size();
}

std::size_t VirtualCanBus::transmitted_frames() const noexcept {
    return transmitted_frames_;
}

std::size_t VirtualCanBus::delivered_frames() const noexcept {
    return delivered_frames_;
}

} // namespace can_sim
