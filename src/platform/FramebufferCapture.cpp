#include "seedsigner_lvgl/platform/FramebufferCapture.hpp"
#include "seedsigner_lvgl/platform/HeadlessDisplay.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace seedsigner::lvgl {

bool FramebufferCapture::write_png(const std::string& path,
                                    const HeadlessDisplay& display) {
    const auto& fb = display.framebuffer();
    if (fb.empty()) return false;

    const auto fw = display.width();
    const auto fh = display.height();

    std::vector<std::uint8_t> rgb(static_cast<std::size_t>(fw) * fh * 3);

    for (std::uint32_t y = 0; y < fh; ++y) {
        for (std::uint32_t x = 0; x < fw; ++x) {
            const std::size_t idx = static_cast<std::size_t>(y) * fw + x;
            const std::uint16_t c = fb[idx].full;
            const std::uint8_t r5 = (c >> 11) & 0x1F;
            const std::uint8_t g6 = (c >> 5) & 0x3F;
            const std::uint8_t b5 = c & 0x1F;

            const std::size_t out = (static_cast<std::size_t>(y) * fw + x) * 3;
            rgb[out + 0] = (r5 << 3) | (r5 >> 2);
            rgb[out + 1] = (g6 << 2) | (g6 >> 4);
            rgb[out + 2] = (b5 << 3) | (b5 >> 2);
        }
    }

    int ok = stbi_write_png(path.c_str(), static_cast<int>(fw), static_cast<int>(fh), 3, rgb.data(), static_cast<int>(fw) * 3);
    return ok != 0;
}

}  // namespace seedsigner::lvgl
