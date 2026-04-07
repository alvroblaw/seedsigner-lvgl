#include "seedsigner_lvgl/screens/PSBTOverviewScreen.hpp"

#include <lvgl.h>

#include "seedsigner_lvgl/contracts/PSBTOverviewContract.hpp"

namespace seedsigner::lvgl {
namespace {

constexpr const char* kNextAction = "next_requested";
constexpr const char* kBackAction = "back_requested";
constexpr const char* kDetailAction = "detail_requested";
constexpr const char* kOverviewComponent = "psbt_overview_screen";

constexpr const char* kInputsLabel = "Inputs";
constexpr const char* kRecipientsLabel = "Recipients";
constexpr const char* kChangeLabel = "Change";
constexpr const char* kFeeLabel = "Fee";
constexpr const char* kOpReturnLabel = "OP_RETURN";

constexpr int kRowHeight = 32;
constexpr int kIconWidth = 24;
constexpr int kArrowWidth = 16;

// Helper to create a row with icon, label, and optional value
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
        lv_obj_set_style_text_color(widgets.value, lv_color_hex(0x888888), 0);
        lv_obj_set_flex_grow(widgets.value, 1);
        lv_obj_set_style_text_align(widgets.value, LV_TEXT_ALIGN_RIGHT, 0);
    } else {
        widgets.value = nullptr;
    }

    return widgets;
}

}  // namespace

void PSBTOverviewScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    params_ = parse_psbt_overview_params(route.args);

    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 16, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Title
    title_label_ = lv_label_create(container_);
    lv_label_set_text(title_label_, "Review Transaction");
    lv_obj_set_style_text_align(title_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title_label_, lv_pct(100));
    lv_obj_set_style_pad_bottom(title_label_, 16, 0);

    // Total amount
    amount_label_ = lv_label_create(container_);
    lv_label_set_text(amount_label_, params_.total_amount.c_str());
    lv_obj_set_style_text_color(amount_label_, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_set_style_text_align(amount_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(amount_label_, lv_pct(100));
    lv_obj_set_style_pad_bottom(amount_label_, 24, 0);

    // Diagram container
    diagram_container_ = lv_obj_create(container_);
    lv_obj_set_size(diagram_container_, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_border_width(diagram_container_, 0, 0);
    lv_obj_set_style_pad_all(diagram_container_, 0, 0);
    lv_obj_set_flex_flow(diagram_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(diagram_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_bottom(diagram_container_, 24, 0);

    // Inputs row
    std::string inputs_text = std::to_string(params_.inputs_count) + (params_.inputs_count == 1 ? " input" : " inputs");
    inputs_row_ = create_row(diagram_container_, LV_SYMBOL_DOWNLOAD, kInputsLabel, inputs_text.c_str()).container;

    // Arrow between inputs and recipients (optional visual)
    lv_obj_t* arrow1 = lv_label_create(diagram_container_);
    lv_label_set_text(arrow1, LV_SYMBOL_DOWN);
    lv_obj_set_style_text_align(arrow1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(arrow1, lv_pct(100));
    lv_obj_set_style_pad_top(arrow1, 4, 0);
    lv_obj_set_style_pad_bottom(arrow1, 4, 0);

    // Recipients row
    std::string recipients_text = std::to_string(params_.outputs_count) + (params_.outputs_count == 1 ? " output" : " outputs");
    recipients_row_ = create_row(diagram_container_, LV_SYMBOL_UPLOAD, kRecipientsLabel, recipients_text.c_str()).container;

    // Change row (optional)
    if (params_.change_amount) {
        lv_obj_t* arrow2 = lv_label_create(diagram_container_);
        lv_label_set_text(arrow2, LV_SYMBOL_RIGHT);
        lv_obj_set_style_text_align(arrow2, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(arrow2, lv_pct(100));
        lv_obj_set_style_pad_top(arrow2, 4, 0);
        lv_obj_set_style_pad_bottom(arrow2, 4, 0);

        change_row_ = create_row(diagram_container_, LV_SYMBOL_LOOP, kChangeLabel, params_.change_amount->c_str()).container;
    } else {
        change_row_ = nullptr;
    }

    // Fee row
    fee_row_ = create_row(diagram_container_, LV_SYMBOL_SETTINGS, kFeeLabel, params_.fee_amount.c_str()).container;

    // OP_RETURN row (optional)
    if (params_.has_op_return) {
        op_return_row_ = create_row(diagram_container_, LV_SYMBOL_FILE, kOpReturnLabel, "Data").container;
    } else {
        op_return_row_ = nullptr;
    }

    // Footer hint
    footer_label_ = lv_label_create(container_);
    lv_label_set_text(footer_label_, "Press OK to continue, BACK to cancel");
    lv_obj_set_style_text_color(footer_label_, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_align(footer_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(footer_label_, lv_pct(100));
    lv_obj_set_style_pad_top(footer_label_, 16, 0);
}

void PSBTOverviewScreen::destroy() {
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
    title_label_ = nullptr;
    amount_label_ = nullptr;
    diagram_container_ = nullptr;
    inputs_row_ = nullptr;
    recipients_row_ = nullptr;
    change_row_ = nullptr;
    fee_row_ = nullptr;
    op_return_row_ = nullptr;
    footer_label_ = nullptr;
    context_ = {};
}

bool PSBTOverviewScreen::handle_input(const InputEvent& input) {
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

void PSBTOverviewScreen::emit_next() {
    context_.emit_action(kNextAction, kOverviewComponent);
}

void PSBTOverviewScreen::emit_back() {
    context_.emit_action(kBackAction, kOverviewComponent);
}

void PSBTOverviewScreen::emit_detail(const std::string& target) {
    PSBTOverviewEvent event;
    event.type = PSBTOverviewEvent::Type::DetailRequested;
    event.detail_target = target;
    context_.emit_action(kDetailAction, kOverviewComponent, encode_psbt_overview_event(event));
}

}  // namespace seedsigner::lvgl