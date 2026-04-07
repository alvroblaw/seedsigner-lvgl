#include "seedsigner_lvgl/screens/PSBTDetailScreen.hpp"

#include <lvgl.h>

#include "seedsigner_lvgl/contracts/PSBTDetailContract.hpp"

namespace seedsigner::lvgl {
namespace {

constexpr const char* kBackAction = "back_requested";
constexpr const char* kViewQRAction = "view_qr_requested";
constexpr const char* kDetailComponent = "psbt_detail_screen";

constexpr const char* kTypeLabel = "Type";
constexpr const char* kIndexLabel = "Index";
constexpr const char* kAddressLabel = "Address";
constexpr const char* kAmountLabel = "Amount";
constexpr const char* kDerivationLabel = "Derivation";
constexpr const char* kPubkeyLabel = "Pubkey";
constexpr const char* kNetworkLabel = "Network";

constexpr int kRowHeight = 28;
constexpr int kLabelWidth = 80;

// Helper to create a row with label and value
void create_row(lv_obj_t* parent, const char* label_text, const char* value_text) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, lv_pct(100), kRowHeight);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* label = lv_label_create(row);
    lv_label_set_text(label, label_text);
    lv_obj_set_width(label, kLabelWidth);
    lv_obj_set_style_text_color(label, lv_color_hex(0x888888), 0);

    lv_obj_t* value = lv_label_create(row);
    lv_label_set_text(value, value_text);
    lv_obj_set_flex_grow(value, 1);
    lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_RIGHT, 0);
}

}  // namespace

void PSBTDetailScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    params_ = parse_psbt_detail_params(route.args);

    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 16, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Title
    title_label_ = lv_label_create(container_);
    lv_label_set_text(title_label_, params_.type == PSBTDetailType::Input ? "Input Details" : "Output Details");
    lv_obj_set_style_text_align(title_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title_label_, lv_pct(100));
    lv_obj_set_style_pad_bottom(title_label_, 16, 0);

    // Content container
    content_container_ = lv_obj_create(container_);
    lv_obj_set_size(content_container_, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_border_width(content_container_, 0, 0);
    lv_obj_set_style_pad_all(content_container_, 0, 0);
    lv_obj_set_flex_flow(content_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_bottom(content_container_, 24, 0);

    // Type row
    create_row(content_container_, kTypeLabel, params_.type == PSBTDetailType::Input ? "Input" : "Output");

    // Index row
    create_row(content_container_, kIndexLabel, std::to_string(params_.index).c_str());

    // Address row (may be long, could truncate)
    create_row(content_container_, kAddressLabel, params_.address.empty() ? "Unknown" : params_.address.c_str());

    // Amount row
    create_row(content_container_, kAmountLabel, params_.amount.c_str());

    // Derivation path row (optional)
    if (params_.derivation_path) {
        create_row(content_container_, kDerivationLabel, params_.derivation_path->c_str());
    }

    // Pubkey row (optional)
    if (params_.pubkey) {
        // Truncate pubkey for display
        std::string pubkey_display = *params_.pubkey;
        if (pubkey_display.length() > 16) {
            pubkey_display = pubkey_display.substr(0, 8) + "..." + pubkey_display.substr(pubkey_display.length() - 8);
        }
        create_row(content_container_, kPubkeyLabel, pubkey_display.c_str());
    }

    // Network row
    create_row(content_container_, kNetworkLabel, params_.network.c_str());

    // Footer hint
    footer_label_ = lv_label_create(container_);
    lv_label_set_text(footer_label_, "Press RIGHT to view QR (placeholder), BACK to return");
    lv_obj_set_style_text_color(footer_label_, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_align(footer_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(footer_label_, lv_pct(100));
    lv_obj_set_style_pad_top(footer_label_, 16, 0);
}

void PSBTDetailScreen::destroy() {
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
    title_label_ = nullptr;
    content_container_ = nullptr;
    footer_label_ = nullptr;
    context_ = {};
}

bool PSBTDetailScreen::handle_input(const InputEvent& input) {
    switch (input.key) {
    case InputKey::Back:
        emit_back();
        return true;
    case InputKey::Right:
        emit_view_qr("address"); // placeholder target
        return true;
    case InputKey::Press:
        // OK could also go back for simplicity
        emit_back();
        return true;
    case InputKey::Up:
    case InputKey::Down:
    case InputKey::Left:
        // Ignore other directional keys
        return false;
    }
    return false;
}

void PSBTDetailScreen::emit_back() {
    context_.emit_action(kBackAction, kDetailComponent);
}

void PSBTDetailScreen::emit_view_qr(const std::string& target) {
    PSBTDetailEvent event;
    event.type = PSBTDetailEvent::Type::ViewQRRequested;
    event.qr_target = target;
    context_.emit_action(kViewQRAction, kDetailComponent, encode_psbt_detail_event(event));
}

}  // namespace seedsigner::lvgl