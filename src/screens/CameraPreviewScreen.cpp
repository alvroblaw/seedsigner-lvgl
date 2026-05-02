#include "seedsigner_lvgl/screens/CameraPreviewScreen.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"

#include <algorithm>
#include "seedsigner_lvgl/components/TopNavBar.hpp"

namespace seedsigner::lvgl {
namespace {
constexpr lv_coord_t kPreviewWidth = 216;
constexpr lv_coord_t kPreviewHeight = 144;
constexpr lv_coord_t kPreviewPadding = 6;

lv_color_t grayscale_to_color(std::uint8_t value) {
    return lv_color_make(value, value, value);
}
}  // namespace

void CameraPreviewScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    // No padding on root container; TopNavBar and content container manage their own spacing.

    // Top navigation bar
    top_nav_bar_ = std::make_unique<TopNavBar>(context_);
    TopNavBarConfig nav_config;
    nav_config.title = title_;
    nav_config.show_back = true;
    nav_config.show_home = false;
    nav_config.show_cancel = false;
    // No custom actions
    top_nav_bar_->set_config(nav_config);
    top_nav_bar_->attach(container_);

    // Content container (everything below the nav bar)
    content_container_ = lv_obj_create(container_);
    lv_obj_set_size(content_container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(content_container_, 12, 0);
    lv_obj_set_flex_flow(content_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_flex_grow(content_container_, 1); // Take remaining height

    // Title removed – shown in TopNavBar

    preview_panel_ = lv_obj_create(content_container_);
    lv_obj_set_size(preview_panel_, lv_pct(100), 180);
    lv_obj_set_style_bg_color(preview_panel_, lv_palette_darken(LV_PALETTE_GREY, 2), 0);
    lv_obj_set_style_pad_all(preview_panel_, kPreviewPadding, 0);

    preview_canvas_buffer_.resize(static_cast<std::size_t>(kPreviewWidth) * static_cast<std::size_t>(kPreviewHeight));
    preview_canvas_ = lv_canvas_create(preview_panel_);
    lv_canvas_set_buffer(preview_canvas_, preview_canvas_buffer_.data(), kPreviewWidth, kPreviewHeight,
                         LV_COLOR_FORMAT_NATIVE);
    lv_obj_center(preview_canvas_);

    frame_label_ = lv_label_create(preview_panel_);
    lv_obj_align(frame_label_, LV_ALIGN_BOTTOM_LEFT, 2, 2);
    lv_obj_set_style_bg_opa(frame_label_, LV_OPA_60, 0);
    lv_obj_set_style_bg_color(frame_label_, seedsigner::lvgl::theme::active_theme().BLACK, 0);
    lv_obj_set_style_text_color(frame_label_, seedsigner::lvgl::theme::active_theme().TEXT_PRIMARY, 0);
    lv_obj_set_style_pad_all(frame_label_, 4, 0);

    status_label_ = lv_label_create(content_container_);
    lv_obj_set_width(status_label_, lv_pct(100));
    lv_label_set_long_mode(status_label_, LV_LABEL_LONG_WRAP);

    apply_data(route.args, true);
    refresh_preview();
    context_.emit_needs_data("camera.frame", "preview_surface");
}

void CameraPreviewScreen::destroy() {
    // TopNavBar must be cleared before its parent container is deleted.
    top_nav_bar_.reset();
    preview_canvas_buffer_.clear();
    if (container_ != nullptr) {
        lv_obj_delete(container_);
        container_ = nullptr;
        content_container_ = nullptr;

        preview_panel_ = nullptr;
        preview_canvas_ = nullptr;
        frame_label_ = nullptr;
        status_label_ = nullptr;
    }
}

bool CameraPreviewScreen::handle_input(const InputEvent& input) {
    // First give the top nav bar a chance to handle the input (e.g., hardware back)
    if (top_nav_bar_ && top_nav_bar_->handle_input(input)) {
        return true;
    }
    switch (input.key) {
    case InputKey::Press:
        return context_.emit_action("capture", "preview_surface",
                                    static_cast<std::int64_t>(frame_sequence_));
    case InputKey::Back:
        return context_.emit_cancel("preview_surface");
    case InputKey::Up:
    case InputKey::Down:
        return false;
    }
    return false;
}

bool CameraPreviewScreen::set_data(const PropertyMap& data) {
    apply_data(data, true);
    return true;
}

bool CameraPreviewScreen::patch_data(const PropertyMap& patch) {
    apply_data(patch, false);
    return true;
}

bool CameraPreviewScreen::push_frame(const CameraFrame& frame) {
    frame_sequence_ = frame.sequence;
    frame_width_ = frame.width;
    frame_height_ = frame.height;
    frame_stride_ = frame.stride;
    frame_bytes_ = frame.pixels.size();
    latest_frame_ = frame.pixels;

    // The producer only lends frame memory for the duration of push_frame(); keep our own copy so
    // LVGL never reads caller-owned pixels after the call returns.
    refresh_preview();
    refresh_labels();
    return true;
}

void CameraPreviewScreen::apply_data(const PropertyMap& data, bool replace) {
    if (replace) {
        title_ = "Scan QR";
        status_ = "Waiting for external frames";
        frame_sequence_ = 0;
        frame_width_ = 0;
        frame_height_ = 0;
        frame_stride_ = 0;
        frame_bytes_ = 0;
        latest_frame_.clear();
    }

    if (const auto title = data.find("title"); title != data.end()) {
        title_ = title->second;
    }
    if (const auto status = data.find("status"); status != data.end()) {
        status_ = status->second;
    }
    refresh_labels();
}

void CameraPreviewScreen::refresh_preview() {
    if (preview_canvas_ == nullptr || preview_canvas_buffer_.empty()) {
        return;
    }

    const lv_color_t panel_bg = lv_palette_darken(LV_PALETTE_GREY, 2);
    lv_canvas_fill_bg(preview_canvas_, panel_bg, LV_OPA_COVER);

    if (frame_width_ == 0 || frame_height_ == 0 || latest_frame_.empty()) {
        return;
    }

    const auto source_stride = frame_stride_ == 0 ? frame_width_ : frame_stride_;
    if (source_stride < frame_width_) {
        return;
    }
    if (latest_frame_.size() < static_cast<std::size_t>(source_stride) * static_cast<std::size_t>(frame_height_)) {
        return;
    }

    const auto target_width = static_cast<std::uint32_t>(kPreviewWidth);
    const auto target_height = static_cast<std::uint32_t>(kPreviewHeight);
    const std::uint32_t scaled_width =
        std::max<std::uint32_t>(1, std::min<std::uint32_t>(target_width, (frame_width_ * target_height) / frame_height_));
    const std::uint32_t scaled_height =
        std::max<std::uint32_t>(1, std::min<std::uint32_t>(target_height, (frame_height_ * target_width) / frame_width_));
    const std::uint32_t x_offset = (target_width - scaled_width) / 2;
    const std::uint32_t y_offset = (target_height - scaled_height) / 2;

    for (std::uint32_t y = 0; y < scaled_height; ++y) {
        const auto src_y = std::min<std::uint32_t>(frame_height_ - 1, (y * frame_height_) / scaled_height);
        for (std::uint32_t x = 0; x < scaled_width; ++x) {
            const auto src_x = std::min<std::uint32_t>(frame_width_ - 1, (x * frame_width_) / scaled_width);
            const auto value = latest_frame_[static_cast<std::size_t>(src_y) * source_stride + src_x];
            lv_canvas_set_px(preview_canvas_, static_cast<int32_t>(x + x_offset), static_cast<int32_t>(y + y_offset),
                             grayscale_to_color(value), LV_OPA_COVER);
        }
    }

    lv_obj_invalidate(preview_canvas_);
}

void CameraPreviewScreen::refresh_labels() {
    // Title is shown in TopNavBar, not a separate label
    if (frame_label_ != nullptr) {
        const std::string text = "#" + std::to_string(frame_sequence_) + "  " +
                                 std::to_string(frame_width_) + "x" + std::to_string(frame_height_) +
                                 "  stride=" + std::to_string(frame_stride_ == 0 ? frame_width_ : frame_stride_);
        lv_label_set_text(frame_label_, text.c_str());
    }
    if (status_label_ != nullptr) {
        lv_label_set_text(status_label_, status_.c_str());
    }
}

}  // namespace seedsigner::lvgl
