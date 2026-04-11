#pragma once

#include <cstdint>
#include <vector>

#include <lvgl.h>

namespace seedsigner::lvgl {

class HeadlessDisplay {
public:
    HeadlessDisplay(std::uint32_t width, std::uint32_t height);

    std::uint32_t width() const noexcept { return width_; }
    std::uint32_t height() const noexcept { return height_; }
    std::uint32_t flush_count() const noexcept { return flush_count_; }

    void refresh_now();

public:
    const std::vector<lv_color_t>& framebuffer() const noexcept { return framebuffer_; }

private:
    static void flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);

    std::uint32_t width_;
    std::uint32_t height_;
    std::uint32_t flush_count_{0};
    lv_disp_draw_buf_t draw_buffer_{};
    lv_disp_drv_t display_driver_{};
    std::vector<lv_color_t> framebuffer_;
};

}  // namespace seedsigner::lvgl
