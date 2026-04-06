#include "seedsigner_lvgl/screens/DireWarningScreen.hpp"

namespace seedsigner::lvgl {

void DireWarningScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    PropertyMap args = route.args;
    args["severity"] = "dire_warning";
    WarningScreen::create(context, {.route_id = route.route_id, .args = std::move(args)});
}

}  // namespace seedsigner::lvgl