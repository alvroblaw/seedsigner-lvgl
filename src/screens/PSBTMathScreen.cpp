#include "seedsigner_lvgl/screens/PSBTMathScreen.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"

#include <lvgl.h>

#include "seedsigner_lvgl/components/TopNavBar.hpp"
#include "seedsigner_lvgl/contracts/PSBTMathContract.hpp"

namespace seedsigner::lvgl {
namespace {

constexpr const char* kNextAction = "next_requested";
constexpr const char* kBackAction = "back_requested";
constexpr const char* kDetailAction = "detail_requested";
constexpr const char* kMathComponent = "psbt_math_screen";

constexpr const char* kInputsLabel = "Inputs";
constexpr const char* kOutputsLabel = "Outputs";
constexpr const char* kChangeLabel = "Change";
constexpr const char* kFeeLabel = "Fee";
constexpr const char* kNetLabel = "Net";

constexpr int kRowHeight = 32;
constexpr int kIconWidth = 24;

// Helper to create a row with icon, label, and value
struct RowWidgets {
    lv_obj_t* container;
    lv_obj_t* icon;
    lv_obj_t* label;
    lv_obj_t* value;
};

RowWidgets create_row(lv_obj_t* parent, const char* icon_symbol, const char* text, const char* value_text = nullptr) {
    RowWidgets widgets;
    widgets.container = lv_obj_create(parent);
    lv_obj_set_size(widgets.container, lv_pct(100), kRowHeight);
    lv_obj_set_style_border_width(widgets.container, 0, 0);
    lv_obj_set_style_pad_all(widgets.container, 0, 0);
    lv_obj_set_flex_flow(widgets.container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(widgets.container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    if (icon_symbol != nullptr) {
        widgets.icon = lv_label_create(widgets.container);
        lv_label_set_text(widgets.icon, icon_symbol);
        lv_obj_set_width(widgets.icon, kIconWidth);
        lv_obj_set_style_text_align(widgets.icon, LV_TEXT_ALIGN_CENTER, 0);
    } else {
        widgets.icon = nullptr;
    }

    widgets.label = lv_label_create(widgets.container);
    lv_label_set_text(widgets.label, text);
    lv_obj_set_style_pad_left(widgets.label, 4, 0);

    if (value_text != nullptr) {
        widgets.value = lv_label_create(widgets.container);
        lv_label_set_text(widgets.value, value_text);
        lv_obj_set_style_pad_left(widgets.value, 8, 0);
        lv_obj_set_style_text_color(widgets.value, seedsigner::lvgl::theme::active_theme().TEXT_SECONDARY, 0);
        lv_obj_set_flex_grow(widgets.value, 1);
        lv_obj_set_style_text_align(widgets.value, LV_TEXT_ALIGN_RIGHT, 0);
    } else {
        widgets.value = nullptr;
    }

    return widgets;
}

}  // namespace

void PSBTMathScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    params_ = parse_psbt_math_params(route.args);

    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // No padding on root container; TopNavBar and content container manage their own spacing.

    // Top navigation bar
    top_nav_bar_ = std::make_unique<TopNavBar>(context_);
    TopNavBarConfig nav_config;
    nav_config.title = "Transaction Math";
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
    lv_obj_set_flex_align(content_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(content_container_, 1); // Take remaining height

    // Diagram container
    lv_obj_t* diagram_container = lv_obj_create(content_container_);
    lv_obj_set_size(diagram_container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_border_width(diagram_container, 0, 0);
    lv_obj_set_style_pad_all(diagram_container, 0, 0);
    lv_obj_set_flex_flow(diagram_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(diagram_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_bottom(diagram_container, 24, 0);

    // Inputs row
    create_row(diagram_container, LV_SYMBOL_DOWNLOAD, kInputsLabel, params_.total_input_amount.c_str());

    // Outputs row
    create_row(diagram_container, LV_SYMBOL_UPLOAD, kOutputsLabel, params_.total_output_amount.c_str());

    // Change row (optional)
    if (params_.change_amount) {
        create_row(diagram_container, LV_SYMBOL_LOOP, kChangeLabel, params_.change_amount->c_str());
    } else {
        create_row(diagram_container, LV_SYMBOL_LOOP, kChangeLabel, "–");
    }

    // Fee row
    create_row(diagram_container, LV_SYMBOL_SETTINGS, kFeeLabel, params_.fee_amount.c_str());

    // Net row (inputs - outputs - change - fee) – should be zero
    // For simplicity we display a zero balance; in real implementation compute difference.
    create_row(diagram_container, LV_SYMBOL_OK, kNetLabel, "0");

    // Footer hint
    footer_label_ = lv_label_create(content_container_);
    lv_label_set_text(footer_label_, "Press OK to continue, BACK to cancel");
    lv_obj_set_style_text_color(footer_label_, seedsigner::lvgl::theme::active_theme().TEXT_SECONDARY, 0);
    lv_obj_set_style_text_align(footer_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(footer_label_, lv_pct(100));
    lv_obj_set_style_pad_top(footer_label_, 16, 0);
}

void PSBTMathScreen::destroy() {
    // TopNavBar must be cleared before its parent container is deleted.
    top_nav_bar_.reset();
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
    content_container_ = nullptr;
    footer_label_ = nullptr;
    context_ = {};
}

bool PSBTMathScreen::handle_input(const InputEvent& input) {
    // First give the top nav bar a chance to handle the input (e.g., hardware back)
    if (top_nav_bar_ && top_nav_bar_->handle_input(input)) {
        return true;
    }
    switch (input.key) {
    case InputKey::Press:
        emit_next();
        return true;
    case InputKey::Back:
        emit_back();
        return true;
    case InputKey::Up:
    case InputKey::Down:
    case InputKey::Left:
    case InputKey::Right:
        // For now, ignore directional keys (could be used for selection in future)
        return false;
    }
    return false;
}

void PSBTMathScreen::emit_next() {
    context_.emit_action(kNextAction, kMathComponent);
}

void PSBTMathScreen::emit_back() {
    context_.emit_action(kBackAction, kMathComponent);
}

void PSBTMathScreen::emit_detail(const std::string& target) {
    PSBTMathEvent event;
    event.type = PSBTMathEvent::Type::DetailRequested;
    event.detail_target = target;
    context_.emit_action(kDetailAction, kMathComponent, encode_psbt_math_event(event));
}

}  // namespace seedsigner::lvgl