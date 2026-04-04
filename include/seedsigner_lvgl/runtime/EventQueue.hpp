#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "seedsigner_lvgl/runtime/Event.hpp"

namespace seedsigner::lvgl {

class EventQueue {
public:
    explicit EventQueue(std::size_t capacity = 16);

    bool push(UiEvent event);
    std::optional<UiEvent> next();
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
