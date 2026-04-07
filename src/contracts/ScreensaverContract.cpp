#include "seedsigner_lvgl/contracts/ScreensaverContract.hpp"

#include <charconv>
#include <optional>
#include <string>
#include <string_view>

namespace seedsigner::lvgl {
namespace {

constexpr const char* kAnimationTypeArg = "animation_type";
constexpr const char* kImagePathArg = "image_path";
constexpr const char* kUpdateIntervalMsArg = "update_interval_ms";
constexpr const char* kShowWakeupOverlayArg = "show_wakeup_overlay";
constexpr const char* kEventTypeArg = "event";

std::optional<int> parse_int(std::string_view str) {
    int value;
    auto result = std::from_chars(str.data(), str.data() + str.size(), value);
    if (result.ec != std::errc{} || result.ptr != str.data() + str.size()) {
        return std::nullopt;
    }
    return value;
}

std::optional<bool> parse_bool(std::string_view str) {
    if (str == "1" || str == "true" || str == "TRUE" || str == "yes") {
        return true;
    }
    if (str == "0" || str == "false" || str == "FALSE" || str == "no") {
        return false;
    }
    return std::nullopt;
}

std::string value_or(const PropertyMap& values, const char* key, const char* fallback = "") {
    const auto it = values.find(key);
    return it == values.end() ? std::string{fallback} : it->second;
}

}  // namespace

PropertyMap make_screensaver_route_args(const ScreensaverParams& params) {
    PropertyMap args;
    if (params.animation_type) {
        args[kAnimationTypeArg] = *params.animation_type;
    }
    if (params.image_path) {
        args[kImagePathArg] = *params.image_path;
    }
    args[kUpdateIntervalMsArg] = std::to_string(params.update_interval_ms);
    args[kShowWakeupOverlayArg] = params.show_wakeup_overlay ? "true" : "false";
    return args;
}

ScreensaverParams parse_screensaver_params(const PropertyMap& args) {
    ScreensaverParams params;
    const auto anim_it = args.find(kAnimationTypeArg);
    if (anim_it != args.end() && !anim_it->second.empty()) {
        params.animation_type = anim_it->second;
    }
    const auto image_it = args.find(kImagePathArg);
    if (image_it != args.end() && !image_it->second.empty()) {
        params.image_path = image_it->second;
    }
    const auto interval_str = value_or(args, kUpdateIntervalMsArg, "100");
    const auto interval = parse_int(interval_str);
    params.update_interval_ms = interval.value_or(100);
    if (params.update_interval_ms < 10) params.update_interval_ms = 10;
    if (params.update_interval_ms > 5000) params.update_interval_ms = 5000;
    const auto overlay_str = value_or(args, kShowWakeupOverlayArg, "true");
    const auto overlay = parse_bool(overlay_str);
    params.show_wakeup_overlay = overlay.value_or(true);
    return params;
}

std::string encode_screensaver_event(const ScreensaverEvent& event) {
    std::string encoded;
    encoded += kEventTypeArg;
    encoded += "=";
    switch (event.type) {
        case ScreensaverEvent::Type::Dismissed:
            encoded += "dismissed";
            break;
    }
    return encoded;
}

}  // namespace seedsigner::lvgl