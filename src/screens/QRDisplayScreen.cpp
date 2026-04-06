#include "seedsigner_lvgl/screens/QRDisplayScreen.hpp"

#include <lvgl.h>
#include <lv_qrcode.h>

namespace seedsigner::lvgl {

namespace {

constexpr const char* kBackAction = "back";
constexpr const char* kBrightnessChangedAction = "brightness_changed";
constexpr const char* kQRDisplayComponent = "qr_display";

constexpr int kQRSize = 200; // pixels, adjust as needed
constexpr int kMinBrightness = 0;
constexpr int kMaxBrightness = 100;
constexpr int kBrightnessStep = 10;

lv_obj_t* create_title_label(lv_obj_t* parent, const std::optional<std::string>& title) {
    if (!title || title->empty()) {
        return nullptr;
    }
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_set_width(label, lv_pct(100));
    lv_label_set_text(label, title->c_str());
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_bottom(label, 16, 0);
    return label;
}

}  // namespace

void QRDisplayScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    params_ = parse_qr_display_params(route.args);
    cached_qr_data_ = params_.qr_data;

    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 16, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Title
    title_label_ = create_title_label(container_, params_.title);
    if (title_label_) {
        lv_obj_set_style_pad_bottom(title_label_, 24, 0);
    }

    // QR widget
    qr_widget_ = lv_qrcode_create(container_, kQRSize, lv_color_black(), lv_color_white());
    if (!qr_widget_) {
        // fallback: create a placeholder label
        lv_obj_t* error = lv_label_create(container_);
        lv_label_set_text(error, "QR error");
        return;
    }
    lv_qrcode_update(qr_widget_, params_.qr_data.c_str(), params_.qr_data.length());
    lv_obj_set_style_pad_bottom(qr_widget_, 24, 0);

    // Brightness overlay (black semi-transparent rectangle covering entire QR)
    brightness_overlay_ = lv_obj_create(qr_widget_);
    lv_obj_set_size(brightness_overlay_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(brightness_overlay_, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(brightness_overlay_, LV_OPA_TRANSP, 0); // updated by update_brightness_overlay
    lv_obj_set_style_border_width(brightness_overlay_, 0, 0);
    lv_obj_set_style_radius(brightness_overlay_, 0, 0);
    lv_obj_clear_flag(brightness_overlay_, LV_OBJ_FLAG_CLICKABLE);
    update_brightness_overlay();
}

void QRDisplayScreen::destroy() {
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
    title_label_ = nullptr;
    qr_widget_ = nullptr;
    brightness_overlay_ = nullptr;
    context_ = {};
    cached_qr_data_.clear();
}

bool QRDisplayScreen::handle_input(const InputEvent& input) {
    switch (input.key) {
    case InputKey::Back:
        context_.emit_action(kBackAction, kQRDisplayComponent);
        return true;
    case InputKey::Up:
        // Increase brightness
        if (params_.brightness < kMaxBrightness) {
            params_.brightness += kBrightnessStep;
            if (params_.brightness > kMaxBrightness) params_.brightness = kMaxBrightness;
            update_brightness_overlay();
            emit_brightness_changed();
        }
        return true;
    case InputKey::Down:
        // Decrease brightness
        if (params_.brightness > kMinBrightness) {
            params_.brightness -= kBrightnessStep;
            if (params_.brightness < kMinBrightness) params_.brightness = kMinBrightness;
            update_brightness_overlay();
            emit_brightness_changed();
        }
        return true;
    case InputKey::Press:
    case InputKey::Left:
    case InputKey::Right:
        // Ignored for now
        return false;
    }
    return false;
}

void QRDisplayScreen::update_brightness_overlay() {
    if (!brightness_overlay_) return;
    // Map brightness 0-100 to opacity 0-255 (LV_OPA_COVER is 255)
    // brightness 100 => opacity 0 (fully transparent)
    // brightness 0 => opacity 255 (fully opaque)
    lv_opa_t opacity = static_cast<lv_opa_t>((100 - params_.brightness) * 255 / 100);
    lv_obj_set_style_bg_opa(brightness_overlay_, opacity, 0);
}

void QRDisplayScreen::update_qr_widget() {
    if (!qr_widget_ || params_.qr_data.empty()) return;
    // Only re-render QR if data changed
    if (params_.qr_data != cached_qr_data_) {
        lv_qrcode_update(qr_widget_, params_.qr_data.c_str(), params_.qr_data.length());
        cached_qr_data_ = params_.qr_data;
    }
}

void QRDisplayScreen::emit_brightness_changed() {
    context_.emit_action(kBrightnessChangedAction,
                         kQRDisplayComponent,
                         EventValue{static_cast<std::int64_t>(params_.brightness)});
}

}  // namespace seedsigner::lvgl