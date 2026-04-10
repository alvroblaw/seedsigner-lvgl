#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "seedsigner_lvgl/runtime/Event.hpp"

namespace seedsigner::lvgl {

/// Bounded FIFO queue for outbound UI events.
///
/// The queue is designed for polling-based retrieval, which is the primary
/// path for external controllers (e.g., future MicroPython bindings).
/// Events are pushed by screens or the runtime and consumed via next().
/// When the queue is full, push() returns false and the event is dropped.
class EventQueue {
public:
    explicit EventQueue(std::size_t capacity = 16);

    /// Attempt to enqueue an event. Returns true on success, false if the queue is full.
    bool push(UiEvent event);
    
    /// Retrieve the next pending event, if any. Returns std::nullopt when the queue is empty.
    std::optional<UiEvent> next();
    
    /// Remove all pending events.
    void clear();

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] std::size_t capacity() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

private:
    std::vector<std::optional<UiEvent>> buffer_;
    std::size_t head_{0};
    std::size_t tail_{0};
    std::size_t size_{0};
};

}  // namespace seedsigner::lvgl
