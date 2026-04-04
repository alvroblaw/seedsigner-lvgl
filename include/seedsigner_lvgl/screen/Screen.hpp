#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include <lvgl.h>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/runtime/Event.hpp"

namespace seedsigner::lvgl {

struct ScreenContext {
    lv_obj_t* root{nullptr};
    RouteId route_id;
    ScreenToken screen_token{0};
    std::function<bool(UiEvent)> emit_event;
    std::function<std::uint64_t()> now_ms;

    bool emit(EventType type,
              std::optional<std::string> component_id = std::nullopt,
              std::optional<std::string> action_id = std::nullopt,
              std::optional<EventValue> value = std::nullopt,
              std::optional<EventMeta> meta = std::nullopt) const {
        if (!emit_event) {
            return false;
        }

        return emit_event(UiEvent{
            .type = type,
            .route_id = route_id,
            .screen_token = screen_token,
            .component_id = std::move(component_id),
            .action_id = std::move(action_id),
            .value = std::move(value),
            .meta = std::move(meta),
            .timestamp_ms = now_ms ? now_ms() : 0,
        });
    }

    bool emit_action(std::string action_id,
                     std::optional<std::string> component_id = std::nullopt,
                     std::optional<EventValue> value = std::nullopt,
                     std::optional<EventMeta> meta = std::nullopt) const {
        return emit(EventType::ActionInvoked, std::move(component_id), std::move(action_id), std::move(value), std::move(meta));
    }

    bool emit_cancel(std::optional<std::string> component_id = std::nullopt,
                     std::optional<EventMeta> meta = std::nullopt) const {
        return emit(EventType::CancelRequested, std::move(component_id), std::nullopt, std::nullopt, std::move(meta));
    }

    bool emit_needs_data(std::string key,
                         std::optional<std::string> component_id = std::nullopt,
                         std::optional<EventValue> value = std::nullopt) const {
        return emit(EventType::NeedsData,
                    std::move(component_id),
                    std::nullopt,
                    std::move(value),
                    EventMeta{std::move(key), true});
    }
};

class Screen {
public:
    virtual ~Screen() = default;

    virtual void create(const ScreenContext& context, const RouteDescriptor& route) = 0;
    virtual void on_activate() {}
    virtual void on_deactivate() {}
    virtual void destroy() {}
};

}  // namespace seedsigner::lvgl
