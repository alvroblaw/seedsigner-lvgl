#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

using EventValue = std::variant<std::monostate, bool, std::int64_t, std::string>;

struct EventMeta {
    std::string key;
    EventValue value;
};

enum class EventType {
    RouteActivated,
    ScreenReady,
    ActionInvoked,
    CancelRequested,
    NeedsData,
    Error,
};

struct UiEvent {
    EventType type;
    RouteId route_id;
    ScreenToken screen_token{0};
    std::optional<std::string> component_id;
    std::optional<std::string> action_id;
    std::optional<EventValue> value;
    std::optional<EventMeta> meta;
    std::uint64_t timestamp_ms{0};
};

}  // namespace seedsigner::lvgl
