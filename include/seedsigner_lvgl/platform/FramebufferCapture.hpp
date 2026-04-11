#pragma once

#include <cstdint>
#include <string>

namespace seedsigner::lvgl {

class HeadlessDisplay;

/// Captures the current framebuffer from a HeadlessDisplay and writes a PNG.
class FramebufferCapture {
public:
    static bool write_png(const std::string& path,
                          const HeadlessDisplay& display);
};

}  // namespace seedsigner::lvgl
