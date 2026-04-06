#pragma once

#include <optional>
#include <string>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

struct QRDisplayParams {
    std::string qr_data;
    std::optional<std::string> title;
    int brightness{100}; // 0–100
    // Note: animated QR fields omitted for first increment
    // bool is_animated{false};
    // std::optional<int> frame_count;
    // std::optional<int> frame_delay_ms;
};

struct QRDisplayEvent {
    enum class Type {
        Back,
        BrightnessChanged,
        // FrameChanged,
    } type;
    // For brightness_changed
    std::optional<int> brightness;
    // For frame_changed
    // std::optional<int> frame_index;
};

PropertyMap make_qr_display_route_args(const QRDisplayParams& params);
QRDisplayParams parse_qr_display_params(const PropertyMap& args);
std::string encode_qr_display_event(const QRDisplayEvent& event);

}  // namespace seedsigner::lvgl