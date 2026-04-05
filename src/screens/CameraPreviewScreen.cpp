#include "seedsigner_lvgl/screens/CameraPreviewScreen.hpp"

namespace seedsigner::lvgl {

void CameraPreviewScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 12, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    title_label_ = lv_label_create(container_);
    lv_obj_set_width(title_label_, lv_pct(100));

    preview_panel_ = lv_obj_create(container_);
    lv_obj_set_size(preview_panel_, lv_pct(100), 180);
    lv_obj_set_style_bg_color(preview_panel_, lv_palette_darken(LV_PALETTE_GREY, 2), 0);

    frame_label_ = lv_label_create(preview_panel_);
    lv_obj_center(frame_label_);

    status_label_ = lv_label_create(container_);
    lv_obj_set_width(status_label_, lv_pct(100));
    lv_label_set_long_mode(status_label_, LV_LABEL_LONG_WRAP);

    apply_data(route.args, true);
    context_.emit_needs_data("camera.frame", "preview_surface");
}

void CameraPreviewScreen::destroy() {
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
        title_label_ = nullptr;
        preview_panel_ = nullptr;
        frame_label_ = nullptr;
        status_label_ = nullptr;
    }
}

bool CameraPreviewScreen::handle_input(const InputEvent& input) {
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
    frame_bytes_ = frame.pixels.size();
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
        frame_bytes_ = 0;
    }

    if (const auto title = data.find("title"); title != data.end()) {
        title_ = title->second;
    }
    if (const auto status = data.find("status"); status != data.end()) {
        status_ = status->second;
    }
    refresh_labels();
}

void CameraPreviewScreen::refresh_labels() {
    if (title_label_ != nullptr) {
        lv_label_set_text(title_label_, title_.c_str());
    }
    if (frame_label_ != nullptr) {
        const std::string text = "frame #" + std::to_string(frame_sequence_) + "\n" +
                                 std::to_string(frame_width_) + "x" + std::to_string(frame_height_) +
                                 "\nbytes=" + std::to_string(frame_bytes_);
        lv_label_set_text(frame_label_, text.c_str());
    }
    if (status_label_ != nullptr) {
        lv_label_set_text(status_label_, status_.c_str());
    }
}

}  // namespace seedsigner::lvgl
