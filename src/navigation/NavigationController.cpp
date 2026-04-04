#include "seedsigner_lvgl/navigation/NavigationController.hpp"

namespace seedsigner::lvgl {

NavigationController::NavigationController(const ScreenRegistry& registry)
    : registry_(registry) {}

std::optional<ActiveRoute> NavigationController::activate(const RouteDescriptor& route) {
    return replace(route);
}

std::optional<ActiveRoute> NavigationController::replace(const RouteDescriptor& route) {
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

    next_screen->create(route);
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

std::optional<ActiveRoute> NavigationController::install(const RouteDescriptor& route) {
    return replace(route);
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
