#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace seedsigner::lvgl {

struct CameraFrame {
    std::uint32_t width{0};
    std::uint32_t height{0};
    std::uint32_t stride{0};
    std::uint64_t sequence{0};
    std::vector<std::uint8_t> pixels;
};

}  // namespace seedsigner::lvgl
