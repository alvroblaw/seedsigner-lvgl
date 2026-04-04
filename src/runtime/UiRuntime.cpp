#include "seedsigner_lvgl/runtime/UiRuntime.hpp"

#include <utility>

namespace seedsigner::lvgl {

UiRuntime::UiRuntime(RuntimeConfig config)
    : config_(std::move(config))
    , navigation_controller_(screen_registry_)
    , event_queue_(config_.event_queue_capacity) {}

UiRuntime::~UiRuntime() {
    display_.reset();
    if (initialized_) {
        lv_deinit();
    }
}

bool UiRuntime::init() {
    if (initialized_) {
        return true;
    }

    lv_init();
    display_ = std::make_unique<HeadlessDisplay>(config_.width, config_.height);
    initialized_ = true;
    return true;
}

std::optional<ActiveRoute> UiRuntime::activate(const RouteDescriptor& route) {
    if (!initialized_) {
        return std::nullopt;
    }

    const auto active = navigation_controller_.activate(route, ScreenContext{.root = lv_scr_act()});
    if (!active) {
        emit(UiEvent{
            .type = EventType::Error,
            .route_id = route.route_id,
            .meta = EventMeta{"reason", std::string{"unknown route"}},
            .timestamp_ms = now_ms_,
        });
        return std::nullopt;
    }

    emit(UiEvent{.type = EventType::RouteActivated,
                 .route_id = active->route_id,
                 .screen_token = active->screen_token,
                 .timestamp_ms = now_ms_});
    emit(UiEvent{.type = EventType::ScreenReady,
                 .route_id = active->route_id,
                 .screen_token = active->screen_token,
                 .timestamp_ms = now_ms_});
    return active;
}

std::optional<ActiveRoute> UiRuntime::replace(const RouteDescriptor& route) {
    return activate(route);
}

std::optional<ActiveRoute> UiRuntime::get_active_route() const noexcept {
    return navigation_controller_.get_active_route();
}

bool UiRuntime::send_input(const InputEvent& input) {
    return initialized_ && navigation_controller_.send_input(input);
}

bool UiRuntime::set_screen_data(const PropertyMap& data) {
    return initialized_ && navigation_controller_.set_active_screen_data(data);
}

bool UiRuntime::patch_screen_data(const PropertyMap& patch) {
    return initialized_ && navigation_controller_.patch_active_screen_data(patch);
}

bool UiRuntime::push_frame(const CameraFrame& frame) {
    return initialized_ && navigation_controller_.push_frame_to_active_screen(frame);
}

bool UiRuntime::emit(UiEvent event) {
    return event_queue_.push(std::move(event));
}

std::optional<UiEvent> UiRuntime::next_event() {
    return event_queue_.next();
}

void UiRuntime::tick(std::uint32_t elapsed_ms) {
    if (!initialized_) {
        return;
    }

    now_ms_ += elapsed_ms;
    lv_tick_inc(elapsed_ms);
    lv_timer_handler();
}

void UiRuntime::refresh_now() {
    if (display_ != nullptr) {
        display_->refresh_now();
    }
}

}  // namespace seedsigner::lvgl
