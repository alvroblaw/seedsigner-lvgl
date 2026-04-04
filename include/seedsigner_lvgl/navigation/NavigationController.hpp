#pragma once

#include <memory>
#include <optional>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/screen/ScreenRegistry.hpp"

namespace seedsigner::lvgl {

class NavigationController {
public:
    explicit NavigationController(const ScreenRegistry& registry);

    std::optional<ActiveRoute> activate(const RouteDescriptor& route);
    std::optional<ActiveRoute> replace(const RouteDescriptor& route);
    std::optional<ActiveRoute> get_active_route() const noexcept;

private:
    struct ScreenSlot {
        ActiveRoute route;
        std::unique_ptr<Screen> screen;
    };

    std::optional<ActiveRoute> install(const RouteDescriptor& route);
    void teardown_active();

    const ScreenRegistry& registry_;
    std::optional<ScreenSlot> active_screen_;
    ScreenToken next_screen_token_{1};
};

}  // namespace seedsigner::lvgl
