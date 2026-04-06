#include "seedsigner_lvgl/contracts/QRDisplayContract.hpp"

#include <charconv>
#include <optional>
#include <string>
#include <string_view>

namespace seedsigner::lvgl {
namespace {

constexpr const char* kQrDataArg = "qr_data";
constexpr const char* kTitleArg = "title";
constexpr const char* kBrightnessArg = "brightness";
constexpr const char* kEventTypeArg = "event";
constexpr const char* kBrightnessValueArg = "brightness_value";

std::optional<int> parse_int(std::string_view str) {
    int value;
    auto result = std::from_chars(str.data(), str.data() + str.size(), value);
    if (result.ec != std::errc{} || result.ptr != str.data() + str.size()) {
        return std::nullopt;
    }
    return value;
}

std::string value_or(const PropertyMap& values, const char* key, const char* fallback = "") {
    const auto it = values.find(key);
    return it == values.end() ? std::string{fallback} : it->second;
}

}  // namespace

PropertyMap make_qr_display_route_args(const QRDisplayParams& params) {
    PropertyMap args;
    args[kQrDataArg] = params.qr_data;
    if (params.title) {
        args[kTitleArg] = *params.title;
    }
    args[kBrightnessArg] = std::to_string(params.brightness);
    return args;
}

QRDisplayParams parse_qr_display_params(const PropertyMap& args) {
    QRDisplayParams params;
    params.qr_data = value_or(args, kQrDataArg);
    const auto title_it = args.find(kTitleArg);
    if (title_it != args.end() && !title_it->second.empty()) {
        params.title = title_it->second;
    }
    const auto brightness_str = value_or(args, kBrightnessArg, "100");
    const auto brightness = parse_int(brightness_str);
    params.brightness = brightness.value_or(100);
    // clamp to 0–100
    if (params.brightness < 0) params.brightness = 0;
    if (params.brightness > 100) params.brightness = 100;
    return params;
}

std::string encode_qr_display_event(const QRDisplayEvent& event) {
    std::string encoded;
    switch (event.type) {
        case QRDisplayEvent::Type::Back:
            encoded += kEventTypeArg;
            encoded += "=back";
            break;
        case QRDisplayEvent::Type::BrightnessChanged:
            encoded += kEventTypeArg;
            encoded += "=brightness_changed;";
            encoded += kBrightnessValueArg;
            encoded += "=";
            encoded += std::to_string(event.brightness.value_or(100));
            break;
    }
    return encoded;
}

}  // namespace seedsigner::lvgl