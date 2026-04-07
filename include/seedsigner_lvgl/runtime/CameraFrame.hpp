#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "seedsigner_lvgl/contracts/CameraContract.hpp"

namespace seedsigner::lvgl {

struct CameraFrame {
    std::uint32_t width{0};
    std::uint32_t height{0};
    std::uint32_t stride{0};
    std::uint64_t sequence{0};
    std::uint64_t timestamp_ms{0};
    CameraFormat format{CameraFormat::Grayscale};
    std::vector<std::uint8_t> pixels;
};

}  // namespace seedsigner::lvgl
