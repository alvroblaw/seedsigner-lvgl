#include "seedsigner_lvgl/screens/ErrorScreen.hpp"

namespace seedsigner::lvgl {

void ErrorScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    // Force severity to error, but allow custom icon if provided.
    PropertyMap args = route.args;
    args["severity"] = "error";
    WarningScreen::create(context, {.route_id = route.route_id, .args = std::move(args)});
}

}  // namespace seedsigner::lvgl