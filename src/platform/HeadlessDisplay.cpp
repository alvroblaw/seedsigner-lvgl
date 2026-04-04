#include "seedsigner_lvgl/platform/HeadlessDisplay.hpp"

namespace seedsigner::lvgl {

HeadlessDisplay::HeadlessDisplay(std::uint32_t width, std::uint32_t height)
    : width_(width), height_(height), framebuffer_(width * height) {
    lv_disp_draw_buf_init(&draw_buffer_, framebuffer_.data(), nullptr, framebuffer_.size());
    lv_disp_drv_init(&display_driver_);
    display_driver_.hor_res = width_;
    display_driver_.ver_res = height_;
    display_driver_.flush_cb = flush_cb;
    display_driver_.draw_buf = &draw_buffer_;
    display_driver_.user_data = this;
    lv_disp_drv_register(&display_driver_);
}

void HeadlessDisplay::refresh_now() {
    lv_refr_now(nullptr);
}

void HeadlessDisplay::flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* /*area*/, lv_color_t* /*color_p*/) {
    auto* self = static_cast<HeadlessDisplay*>(disp_drv->user_data);
    if (self != nullptr) {
        ++self->flush_count_;
    }

    lv_disp_flush_ready(disp_drv);
}

}  // namespace seedsigner::lvgl
