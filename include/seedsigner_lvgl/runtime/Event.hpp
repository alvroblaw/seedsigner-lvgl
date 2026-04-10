#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

/// Variant type for event payload values.
/// Kept simple to ease future MicroPython binding generation.
using EventValue = std::variant<std::monostate, bool, std::int64_t, std::string>;

/// Optional key‑value metadata attached to an event.
struct EventMeta {
    std::string key;
    EventValue value;
};

/// Minimum set of event types needed for early runtime work.
enum class EventType {
    RouteActivated,   ///< A route has been successfully activated.
    ScreenReady,      ///< The screen's widget tree is built and ready for interaction.
    ActionInvoked,    ///< A user‑driven action (button press, menu selection, etc.).
    CancelRequested,  ///< User requested cancellation of the current operation.
    NeedsData,        ///< Screen needs data that the host should provide.
    Error,            ///< An error occurred (e.g., unknown route).
};

/// Structured UI event emitted by the runtime or by screens.
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
