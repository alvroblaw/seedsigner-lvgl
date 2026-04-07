#include "seedsigner_lvgl/contracts/StartupSplashContract.hpp"

#include <charconv>
#include <optional>
#include <string>
#include <string_view>

namespace seedsigner::lvgl {
namespace {

constexpr const char* kVersionArg = "version";
constexpr const char* kLogoPathArg = "logo_path";
constexpr const char* kTimeoutMsArg = "timeout_ms";
constexpr const char* kSkipOnInputArg = "skip_on_input";
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

PropertyMap make_startup_splash_route_args(const StartupSplashParams& params) {
    PropertyMap args;
    args[kVersionArg] = params.version;
    if (params.logo_path) {
        args[kLogoPathArg] = *params.logo_path;
    }
    args[kTimeoutMsArg] = std::to_string(params.timeout_ms);
    args[kSkipOnInputArg] = params.skip_on_input ? "true" : "false";
    return args;
}

StartupSplashParams parse_startup_splash_params(const PropertyMap& args) {
    StartupSplashParams params;
    params.version = value_or(args, kVersionArg);
    const auto logo_it = args.find(kLogoPathArg);
    if (logo_it != args.end() && !logo_it->second.empty()) {
        params.logo_path = logo_it->second;
    }
    const auto timeout_str = value_or(args, kTimeoutMsArg, "3000");
    const auto timeout = parse_int(timeout_str);
    params.timeout_ms = timeout.value_or(3000);
    // clamp to sensible range
    if (params.timeout_ms < 0) params.timeout_ms = 0;
    if (params.timeout_ms > 60000) params.timeout_ms = 60000;
    const auto skip_str = value_or(args, kSkipOnInputArg, "true");
    const auto skip = parse_bool(skip_str);
    params.skip_on_input = skip.value_or(true);
    return params;
}

std::string encode_startup_splash_event(const StartupSplashEvent& event) {
    std::string encoded;
    encoded += kEventTypeArg;
    encoded += "=";
    switch (event.type) {
        case StartupSplashEvent::Type::Completed:
            encoded += "completed";
            break;
        case StartupSplashEvent::Type::Skipped:
            encoded += "skipped";
            break;
    }
    return encoded;
}

}  // namespace seedsigner::lvgl