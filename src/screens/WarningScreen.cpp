#include "seedsigner_lvgl/screens/WarningScreen.hpp"

#include <lvgl.h>

namespace seedsigner::lvgl {

namespace {

constexpr const char* kButtonPressedAction = "button_pressed";
constexpr const char* kBackAction = "back";
constexpr const char* kWarningComponent = "warning_screen";

constexpr const char* kTitleArg = "title";
constexpr const char* kBodyArg = "body";
constexpr const char* kButtonTextArg = "button_text";
constexpr const char* kSeverityArg = "severity";
constexpr const char* kIconArg = "icon";

constexpr const char* kDefaultButtonText = "OK";

const char* value_or(const PropertyMap& values, const char* key, const char* fallback) {
    const auto it = values.find(key);
    return it == values.end() ? fallback : it->second.c_str();
}

}  // namespace

void WarningScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    title_ = value_or(route.args, kTitleArg, "");
    body_ = value_or(route.args, kBodyArg, "");
    button_text_ = value_or(route.args, kButtonTextArg, kDefaultButtonText);

    const std::string severity_str = value_or(route.args, kSeverityArg, "warning");
    severity_ = parse_severity(severity_str);

    // If custom icon is provided, use it; otherwise use severity-based icon.
    const std::string custom_icon = value_or(route.args, kIconArg, "");
    const char* icon_text = custom_icon.empty() ? severity_to_icon(severity_) : custom_icon.c_str();

    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 16, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Icon
    icon_label_ = lv_label_create(container_);
    lv_label_set_text(icon_label_, icon_text);
    // icon uses default font
    lv_obj_set_style_text_color(icon_label_, severity_to_title_color(severity_), 0);
    lv_obj_set_style_pad_bottom(icon_label_, 16, 0);

    // Title
    if (!title_.empty()) {
        title_label_ = lv_label_create(container_);
        lv_obj_set_width(title_label_, lv_pct(100));
        lv_label_set_text(title_label_, title_.c_str());
        // title uses default font
        lv_obj_set_style_text_color(title_label_, severity_to_title_color(severity_), 0);
        lv_obj_set_style_text_align(title_label_, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_bottom(title_label_, 8, 0);
    }

    // Body
    if (!body_.empty()) {
        body_label_ = lv_label_create(container_);
        lv_obj_set_width(body_label_, lv_pct(100));
        lv_label_set_long_mode(body_label_, LV_LABEL_LONG_WRAP);
        lv_label_set_text(body_label_, body_.c_str());
        // body uses default font
        lv_obj_set_style_text_align(body_label_, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_bottom(body_label_, 24, 0);
    }

    // Button
    button_ = lv_btn_create(container_);
    lv_obj_set_width(button_, lv_pct(80));
    lv_obj_set_height(button_, 48);
    lv_obj_set_style_radius(button_, 8, 0);
    lv_obj_add_event_cb(button_, [](lv_event_t* e) {
        auto* screen = static_cast<WarningScreen*>(lv_event_get_user_data(e));
        if (screen != nullptr) {
            screen->context_.emit_action(kButtonPressedAction, kWarningComponent);
        }
    }, LV_EVENT_CLICKED, this);

    button_label_ = lv_label_create(button_);
    lv_label_set_text(button_label_, button_text_.c_str());
    lv_obj_center(button_label_);
}

void WarningScreen::destroy() {
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
    icon_label_ = nullptr;
    title_label_ = nullptr;
    body_label_ = nullptr;
    button_ = nullptr;
    button_label_ = nullptr;
    context_ = {};
}

bool WarningScreen::handle_input(const InputEvent& input) {
    switch (input.key) {
    case InputKey::Press:
        // Treat hardware OK button as pressing the on-screen button.
        context_.emit_action(kButtonPressedAction, kWarningComponent);
        return true;
    case InputKey::Back:
        context_.emit_action(kBackAction, kWarningComponent);
        return true;
    case InputKey::Up:
    case InputKey::Down:
    case InputKey::Left:
    case InputKey::Right:
        // Ignore directional keys for this screen.
        return false;
    }
    return false;
}

WarningSeverity WarningScreen::parse_severity(const std::string& severity_str) {
    if (severity_str == "error") {
        return WarningSeverity::Error;
    }
    if (severity_str == "dire_warning") {
        return WarningSeverity::DireWarning;
    }
    // Default to warning
    return WarningSeverity::Warning;
}

const char* WarningScreen::severity_to_icon(WarningSeverity severity) {
    switch (severity) {
    case WarningSeverity::Error:
        return LV_SYMBOL_CLOSE;
    case WarningSeverity::DireWarning:
        return LV_SYMBOL_WARNING;
    case WarningSeverity::Warning:
    default:
        return LV_SYMBOL_WARNING;
    }
}

lv_color_t WarningScreen::severity_to_title_color(WarningSeverity severity) {
    switch (severity) {
    case WarningSeverity::Error:
        return lv_palette_main(LV_PALETTE_RED);
    case WarningSeverity::DireWarning:
        return lv_color_make(0xFF, 0x66, 0x00); // Orange-red
    case WarningSeverity::Warning:
    default:
        return lv_palette_main(LV_PALETTE_YELLOW);
    }
}

const char* WarningScreen::default_button_text(WarningSeverity severity) {
    // Could vary by severity if desired.
    return kDefaultButtonText;
}

}  // namespace seedsigner::lvgl