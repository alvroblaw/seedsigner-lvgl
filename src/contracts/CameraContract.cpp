#include "seedsigner_lvgl/contracts/CameraContract.hpp"

#include <charconv>
#include <optional>
#include <string>
#include <string_view>

namespace seedsigner::lvgl {
namespace {

constexpr const char* kFormatArg = "format";
constexpr const char* kWidthArg = "width";
constexpr const char* kHeightArg = "height";
constexpr const char* kFpsArg = "fps";
constexpr const char* kBufferCountArg = "buffer_count";

// Format string values
constexpr const char* kFormatRGB565 = "rgb565";
constexpr const char* kFormatGrayscale = "grayscale";
constexpr const char* kFormatJPEG = "jpeg";

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

PropertyMap make_camera_route_args(const CameraParams& params) {
    PropertyMap args;
    args[kFormatArg] = to_string(params.desired_format);
    if (params.desired_width > 0) {
        args[kWidthArg] = std::to_string(params.desired_width);
    }
    if (params.desired_height > 0) {
        args[kHeightArg] = std::to_string(params.desired_height);
    }
    if (params.max_fps > 0) {
        args[kFpsArg] = std::to_string(params.max_fps);
    }
    if (params.buffer_count != 1) {
        args[kBufferCountArg] = std::to_string(params.buffer_count);
    }
    return args;
}

CameraParams parse_camera_params(const PropertyMap& args) {
    CameraParams params;
    const std::string format_str = value_or(args, kFormatArg, kFormatGrayscale);
    params.desired_format = parse_camera_format(format_str);

    const auto width_str = value_or(args, kWidthArg, "0");
    const auto width = parse_int(width_str);
    params.desired_width = width.value_or(0);

    const auto height_str = value_or(args, kHeightArg, "0");
    const auto height = parse_int(height_str);
    params.desired_height = height.value_or(0);

    const auto fps_str = value_or(args, kFpsArg, "0");
    const auto fps = parse_int(fps_str);
    params.max_fps = fps.value_or(0);

    const auto buffer_str = value_or(args, kBufferCountArg, "1");
    const auto buffer = parse_int(buffer_str);
    params.buffer_count = buffer.value_or(1);
    // Clamp to reasonable range
    if (params.buffer_count < 1) params.buffer_count = 1;
    if (params.buffer_count > 10) params.buffer_count = 10;

    return params;
}

std::string to_string(CameraFormat format) {
    switch (format) {
        case CameraFormat::RGB565:
            return kFormatRGB565;
        case CameraFormat::Grayscale:
            return kFormatGrayscale;
        case CameraFormat::JPEG:
            return kFormatJPEG;
    }
    return kFormatGrayscale;
}

CameraFormat parse_camera_format(std::string_view raw) {
    // case‑insensitive comparison
    std::string lower;
    lower.reserve(raw.size());
    for (char c : raw) {
        lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    if (lower == kFormatRGB565) {
        return CameraFormat::RGB565;
    }
    if (lower == kFormatJPEG) {
        return CameraFormat::JPEG;
    }
    // default to grayscale
    return CameraFormat::Grayscale;
}

}  // namespace seedsigner::lvgl