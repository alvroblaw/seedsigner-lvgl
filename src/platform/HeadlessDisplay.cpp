#include "seedsigner_lvgl/platform/HeadlessDisplay.hpp"

#include <cstring>

namespace seedsigner::lvgl {

// 2 bytes per pixel for RGB565 (LV_COLOR_DEPTH = 16).
static constexpr std::size_t kBytesPerPixel = 2;

HeadlessDisplay::HeadlessDisplay(std::uint32_t width, std::uint32_t height)
    : width_(width)
    , height_(height)
    , draw_buf_(static_cast<std::size_t>(width) * 40 * kBytesPerPixel)
    , framebuffer_(static_cast<std::size_t>(width) * height * kBytesPerPixel, 0)
{
    display_ = lv_display_create(static_cast<int32_t>(width_),
                                 static_cast<int32_t>(height_));
    lv_display_set_user_data(display_, this);
    lv_display_set_flush_cb(display_, flush_cb);
    lv_display_set_buffers(display_,
                            draw_buf_.data(), nullptr,
                            static_cast<uint32_t>(draw_buf_.size()),
                            LV_DISPLAY_RENDER_MODE_PARTIAL);
}

HeadlessDisplay::~HeadlessDisplay() {
    if (display_) {
        lv_display_delete(display_);
        display_ = nullptr;
    }
}

void HeadlessDisplay::refresh_now() {
    lv_refr_now(display_);
}

void HeadlessDisplay::flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    auto* self = static_cast<HeadlessDisplay*>(lv_display_get_user_data(disp));
    if (self != nullptr) {
        ++self->flush_count_;

        const int area_w = area->x2 - area->x1 + 1;
        const std::size_t row_bytes = static_cast<std::size_t>(area_w) * kBytesPerPixel;

        for (int y = area->y1; y <= area->y2; ++y) {
            const std::size_t dst_offset =
                (static_cast<std::size_t>(y) * self->width_ + static_cast<std::size_t>(area->x1))
                * kBytesPerPixel;
            std::memcpy(self->framebuffer_.data() + dst_offset, px_map, row_bytes);
            px_map += row_bytes;
        }
    }

    lv_display_flush_ready(disp);
}

}  // namespace seedsigner::lvgl
