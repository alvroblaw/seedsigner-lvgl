#pragma once

#include <memory>
#include <optional>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/runtime/InputEvent.hpp"
#include "seedsigner_lvgl/screen/ScreenRegistry.hpp"

namespace seedsigner::lvgl {

class NavigationController {
public:
    explicit NavigationController(const ScreenRegistry& registry);

    std::optional<ActiveRoute> activate(const RouteDescriptor& route, const ScreenContext& context);
    std::optional<ActiveRoute> replace(const RouteDescriptor& route, const ScreenContext& context);
    std::optional<ActiveRoute> get_active_route() const noexcept;
    bool send_input(const InputEvent& input);

private:
    void teardown_active();

    struct ScreenSlot {
        ActiveRoute route;
        std::unique_ptr<Screen> screen;
    };

    const ScreenRegistry& registry_;
    std::optional<ScreenSlot> active_screen_;
    ScreenToken next_screen_token_{1};
};

}  // namespace seedsigner::lvgl
