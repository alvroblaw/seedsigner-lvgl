#include "seedsigner_lvgl/runtime/EventQueue.hpp"

namespace seedsigner::lvgl {

EventQueue::EventQueue(std::size_t capacity)
    : buffer_(capacity ? capacity : 1) {}

bool EventQueue::push(UiEvent event) {
    if (size_ >= buffer_.size()) {
        return false;
    }

    buffer_[tail_] = std::move(event);
    tail_ = (tail_ + 1) % buffer_.size();
    ++size_;
    return true;
}

std::optional<UiEvent> EventQueue::next() {
    if (size_ == 0) {
        return std::nullopt;
    }

    auto event = std::move(buffer_[head_]);
    buffer_[head_].reset();
    head_ = (head_ + 1) % buffer_.size();
    --size_;
    return event;
}

void EventQueue::clear() {
    for (auto& slot : buffer_) {
        slot.reset();
    }
    head_ = 0;
    tail_ = 0;
    size_ = 0;
}

std::size_t EventQueue::size() const noexcept { return size_; }
std::size_t EventQueue::capacity() const noexcept { return buffer_.size(); }
bool EventQueue::empty() const noexcept { return size_ == 0; }

}  // namespace seedsigner::lvgl
