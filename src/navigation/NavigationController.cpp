#include "seedsigner_lvgl/navigation/NavigationController.hpp"

namespace seedsigner::lvgl {

NavigationController::NavigationController(const ScreenRegistry& registry)
    : registry_(registry) {}

std::optional<ActiveRoute> NavigationController::activate(const RouteDescriptor& route, const ScreenContext& context) {
    return replace(route, context);
}

std::optional<ActiveRoute> NavigationController::replace(const RouteDescriptor& route, const ScreenContext& context) {
    auto next_screen = registry_.create(route.route_id);
    if (!next_screen) {
        return std::nullopt;
    }

    teardown_active();

    const ActiveRoute active_route{
        .route_id = route.route_id,
        .screen_token = next_screen_token_++,
        .stack_depth = 1,
    };

    auto screen_context = context;
    screen_context.route_id = active_route.route_id;
    screen_context.screen_token = active_route.screen_token;

    next_screen->create(screen_context, route);
    next_screen->on_activate();
    active_screen_ = ScreenSlot{.route = active_route, .screen = std::move(next_screen)};
    return active_route;
}

std::optional<ActiveRoute> NavigationController::get_active_route() const noexcept {
    if (!active_screen_) {
        return std::nullopt;
    }

    return active_screen_->route;
}

bool NavigationController::send_input(const InputEvent& input) {
    return active_screen_ && active_screen_->screen->handle_input(input);
}

bool NavigationController::set_active_screen_data(const PropertyMap& data) {
    return active_screen_ && active_screen_->screen->set_data(data);
}

bool NavigationController::patch_active_screen_data(const PropertyMap& patch) {
    return active_screen_ && active_screen_->screen->patch_data(patch);
}

bool NavigationController::push_frame_to_active_screen(const CameraFrame& frame) {
    return active_screen_ && active_screen_->screen->push_frame(frame);
}

void NavigationController::teardown_active() {
    if (!active_screen_) {
        return;
    }

    active_screen_->screen->on_deactivate();
    active_screen_->screen->destroy();
    active_screen_.reset();
}

}  // namespace seedsigner::lvgl
