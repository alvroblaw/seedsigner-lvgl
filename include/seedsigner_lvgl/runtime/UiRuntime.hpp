#pragma once

#include <optional>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/navigation/NavigationController.hpp"
#include "seedsigner_lvgl/screen/ScreenRegistry.hpp"

namespace seedsigner::lvgl {

class UiRuntime {
public:
    UiRuntime();

    ScreenRegistry& screen_registry() noexcept { return screen_registry_; }
    const ScreenRegistry& screen_registry() const noexcept { return screen_registry_; }

    std::optional<ActiveRoute> activate(const RouteDescriptor& route);
    std::optional<ActiveRoute> replace(const RouteDescriptor& route);
    std::optional<ActiveRoute> get_active_route() const noexcept;

private:
    ScreenRegistry screen_registry_;
    NavigationController navigation_controller_;
};

}  // namespace seedsigner::lvgl
