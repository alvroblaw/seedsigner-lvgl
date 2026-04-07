#pragma once

#include <optional>
#include <string>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

struct StartupSplashParams {
    std::string version;
    std::optional<std::string> logo_path;
    int timeout_ms{3000}; // 0 means no auto‑complete
    bool skip_on_input{true};
};

struct StartupSplashEvent {
    enum class Type {
        Completed, // auto‑completed after timeout
        Skipped,   // user input before timeout
    } type;
};

PropertyMap make_startup_splash_route_args(const StartupSplashParams& params);
StartupSplashParams parse_startup_splash_params(const PropertyMap& args);
std::string encode_startup_splash_event(const StartupSplashEvent& event);

}  // namespace seedsigner::lvgl