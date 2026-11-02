#pragma once

#include "can/frame.hpp"

#include <cstddef>
#include <deque>
#include <functional>
#include <vector>

namespace can_sim {

class VirtualCanBus {
public:
    using Receiver = std::function<void(const CanFrame&)>;

    void subscribe(Receiver receiver);
    void transmit(const CanFrame& frame);
    std::size_t dispatch();

    std::size_t pending_frames() const noexcept;
    std::size_t transmitted_frames() const noexcept;
    std::size_t delivered_frames() const noexcept;

private:
    std::vector<Receiver> receivers_;
    std::deque<CanFrame> queue_;
    std::size_t transmitted_frames_{0};
    std::size_t delivered_frames_{0};
};

} // namespace can_sim
