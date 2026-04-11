#include "seedsigner_lvgl/screens/WarningScreen.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"
#include "seedsigner_lvgl/components/TopNavBar.hpp"
#include "icons.h"

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
    const lv_img_dsc_t* icon_img = nullptr;
    const char* icon_symbol = nullptr;
    if (custom_icon.empty()) {
        icon_img = severity_to_icon(severity_);
    } else {
        icon_symbol = custom_icon.c_str();
    }

    // Determine TopNavBar title: use custom title if provided, else default based on severity
    std::string nav_title = title_;
    if (nav_title.empty()) {
        switch (severity_) {
            case WarningSeverity::Error:
                nav_title = "Error";
                break;
            case WarningSeverity::DireWarning:
                nav_title = "Warning";
                break;
            case WarningSeverity::Warning:
            default:
                nav_title = "Warning";
                break;
        }
    }

    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // No padding on root container; TopNavBar and content container manage their own spacing.

    // Top navigation bar
    top_nav_bar_ = std::make_unique<TopNavBar>(context_);
    TopNavBarConfig nav_config;
    nav_config.title = nav_title;
    nav_config.show_back = true;
    nav_config.show_home = false;
    nav_config.show_cancel = false;
    // No custom actions
    top_nav_bar_->set_config(nav_config);
    top_nav_bar_->attach(container_);

    // Content container (everything below the nav bar)
    content_container_ = lv_obj_create(container_);
    lv_obj_set_size(content_container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(content_container_, 16, 0);
    lv_obj_set_flex_flow(content_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_container_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(content_container_, 1); // Take remaining height

    // Apply warning style with severity color
    seedsigner::lvgl::theme::apply_warning_style(content_container_, severity_to_title_color(severity_));
    // Reapply desired padding (overrides theme's COMPONENT_PADDING)
    lv_obj_set_style_pad_all(content_container_, 16, 0);

    // Icon
    if (icon_img) {
        icon_obj_ = lv_img_create(content_container_);
        lv_img_set_src(icon_obj_, icon_img);
    } else {
        icon_obj_ = lv_label_create(content_container_);
        lv_label_set_text(icon_obj_, icon_symbol);
        lv_obj_set_style_text_color(icon_obj_, severity_to_title_color(severity_), 0);
    }
    lv_obj_set_style_pad_bottom(icon_obj_, 16, 0);

    // Body
    if (!body_.empty()) {
        body_label_ = lv_label_create(content_container_);
        lv_obj_set_width(body_label_, lv_pct(100));
        lv_label_set_long_mode(body_label_, LV_LABEL_LONG_WRAP);
        lv_label_set_text(body_label_, body_.c_str());
        // body uses default font
        lv_obj_set_style_text_align(body_label_, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_bottom(body_label_, 24, 0);
    }

    // Button
    button_ = lv_btn_create(content_container_);
    lv_obj_set_width(button_, lv_pct(80));
    lv_obj_set_height(button_, 48);
    seedsigner::lvgl::theme::apply_button_style(button_, true); // Primary button
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
    // TopNavBar must be cleared before its parent container is deleted.
    top_nav_bar_.reset();
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
        content_container_ = nullptr;
        icon_obj_ = nullptr;
        body_label_ = nullptr;
        button_ = nullptr;
        button_label_ = nullptr;
    }
    context_ = {};
}

bool WarningScreen::handle_input(const InputEvent& input) {
    // First give the top nav bar a chance to handle the input (e.g., hardware back)
    if (top_nav_bar_ && top_nav_bar_->handle_input(input)) {
        return true;
    }
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

const lv_img_dsc_t* WarningScreen::severity_to_icon(WarningSeverity severity) {
    switch (severity) {
    case WarningSeverity::Error:
        return &img_warning; // Use warning icon for error (colored red via title)
    case WarningSeverity::DireWarning:
        return &img_dire_warning;
    case WarningSeverity::Warning:
    default:
        return &img_warning;
    }
}

lv_color_t WarningScreen::severity_to_title_color(WarningSeverity severity) {
    switch (severity) {
    case WarningSeverity::Error:
        return seedsigner::lvgl::theme::colors::ERROR;
    case WarningSeverity::DireWarning:
        return seedsigner::lvgl::theme::colors::WARNING; // Orange-amber
    case WarningSeverity::Warning:
    default:
        return seedsigner::lvgl::theme::colors::WARNING; // Amber for both warning levels
    }
}

const char* WarningScreen::default_button_text(WarningSeverity severity) {
    // Could vary by severity if desired.
    return kDefaultButtonText;
}

}  // namespace seedsigner::lvgl