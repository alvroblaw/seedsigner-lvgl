#pragma once

#include <cstdint>
#include <vector>

#include <lvgl.h>

namespace seedsigner::lvgl {

/// Headless (off-screen) display adapter for the LVGL host build.
/// Uses the LVGL v9 display API: lv_display_create / set_draw_buffers / flush_cb.
class HeadlessDisplay {
public:
    HeadlessDisplay(std::uint32_t width, std::uint32_t height);
    ~HeadlessDisplay();

    std::uint32_t width()  const noexcept { return width_; }
    std::uint32_t height() const noexcept { return height_; }
    std::uint32_t flush_count() const noexcept { return flush_count_; }

    void refresh_now();

    /// Raw framebuffer bytes — RGB565 pixels (2 bytes each), row-major.
    const std::vector<std::uint8_t>& framebuffer() const noexcept { return framebuffer_; }

private:
    static void flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map);

    std::uint32_t width_;
    std::uint32_t height_;
    std::uint32_t flush_count_{0};
    lv_display_t* display_{nullptr};
    std::vector<std::uint8_t> draw_buf_;    ///< LVGL render scratch (partial rows)
    std::vector<std::uint8_t> framebuffer_; ///< Full-frame buffer (always coherent)
};

}  // namespace seedsigner::lvgl
