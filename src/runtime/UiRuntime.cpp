#include "seedsigner_lvgl/runtime/UiRuntime.hpp"

namespace seedsigner::lvgl {

UiRuntime::UiRuntime()
    : navigation_controller_(screen_registry_) {}

std::optional<ActiveRoute> UiRuntime::activate(const RouteDescriptor& route) {
    return navigation_controller_.activate(route);
}

std::optional<ActiveRoute> UiRuntime::replace(const RouteDescriptor& route) {
    return navigation_controller_.replace(route);
}

std::optional<ActiveRoute> UiRuntime::get_active_route() const noexcept {
    return navigation_controller_.get_active_route();
}

}  // namespace seedsigner::lvgl
