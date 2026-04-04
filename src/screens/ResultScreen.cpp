#include "seedsigner_lvgl/screens/ResultScreen.hpp"

namespace seedsigner::lvgl {

void ResultScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 12, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    title_label_ = lv_label_create(container_);
    lv_obj_set_width(title_label_, lv_pct(100));

    body_label_ = lv_label_create(container_);
    lv_obj_set_width(body_label_, lv_pct(100));
    lv_label_set_long_mode(body_label_, LV_LABEL_LONG_WRAP);

    apply_data(route.args, true);
}

void ResultScreen::destroy() {
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
        title_label_ = nullptr;
        body_label_ = nullptr;
    }
}

bool ResultScreen::handle_input(const InputEvent& input) {
    switch (input.key) {
    case InputKey::Press:
        return context_.emit_action(continue_action_, "result_screen");
    case InputKey::Back:
        return context_.emit_cancel("result_screen");
    case InputKey::Up:
    case InputKey::Down:
        return false;
    }
    return false;
}

bool ResultScreen::set_data(const PropertyMap& data) {
    apply_data(data, true);
    return true;
}

bool ResultScreen::patch_data(const PropertyMap& patch) {
    apply_data(patch, false);
    return true;
}

void ResultScreen::apply_data(const PropertyMap& data, bool replace) {
    if (replace) {
        title_ = "Scan Result";
        body_ = "No result yet.";
        continue_action_ = "continue";
    }

    if (const auto title = data.find("title"); title != data.end()) {
        title_ = title->second;
    }
    if (const auto body = data.find("body"); body != data.end()) {
        body_ = body->second;
    }
    if (const auto action = data.find("continue_action"); action != data.end()) {
        continue_action_ = action->second;
    }
    refresh_labels();
}

void ResultScreen::refresh_labels() {
    if (title_label_ != nullptr) {
        lv_label_set_text(title_label_, title_.c_str());
    }
    if (body_label_ != nullptr) {
        lv_label_set_text(body_label_, body_.c_str());
    }
}

}  // namespace seedsigner::lvgl
