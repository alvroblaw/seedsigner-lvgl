#pragma once

#include <optional>
#include <string>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

struct ScreensaverParams {
    std::optional<std::string> animation_type; // e.g., "slide", "fade", "logo_bounce"
    std::optional<std::string> image_path;     // static image if animation_type empty
    int update_interval_ms{100};               // animation frame interval
    bool show_wakeup_overlay{true};            // show "Touch to continue" overlay
};

struct ScreensaverEvent {
    enum class Type {
        Dismissed, // user input dismissed the screensaver
    } type;
};

PropertyMap make_screensaver_route_args(const ScreensaverParams& params);
ScreensaverParams parse_screensaver_params(const PropertyMap& args);
std::string encode_screensaver_event(const ScreensaverEvent& event);

}  // namespace seedsigner::lvgl