#pragma once

#include <lvgl.h>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/runtime/CameraFrame.hpp"
#include "seedsigner_lvgl/runtime/Event.hpp"
#include "seedsigner_lvgl/runtime/InputEvent.hpp"

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
              std::optional<EventMeta> meta = std::nullopt) const;
    bool emit_action(std::string action_id,
                     std::optional<std::string> component_id = std::nullopt,
                     std::optional<EventValue> value = std::nullopt,
                     std::optional<EventMeta> meta = std::nullopt) const;
    bool emit_cancel(std::optional<std::string> component_id = std::nullopt,
                     std::optional<EventMeta> meta = std::nullopt) const;
    bool emit_needs_data(std::string key,
                         std::optional<std::string> component_id = std::nullopt,
                         std::optional<EventValue> value = std::nullopt) const;
};

class Screen {
public:
    virtual ~Screen() = default;

    virtual void create(const ScreenContext& context, const RouteDescriptor& route) = 0;
    virtual void on_activate() {}
    virtual void on_deactivate() {}
    virtual void destroy() {}
    virtual bool handle_input(const InputEvent&) { return false; }
    virtual bool set_data(const PropertyMap&) { return false; }
    virtual bool patch_data(const PropertyMap&) { return false; }
    virtual bool push_frame(const CameraFrame&) { return false; }
};

}  // namespace seedsigner::lvgl
