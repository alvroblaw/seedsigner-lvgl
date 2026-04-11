#include "seedsigner_lvgl/screens/PSBTDetailScreen.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"

#include <lvgl.h>

#include "seedsigner_lvgl/components/TopNavBar.hpp"
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
    lv_obj_set_style_text_color(label, seedsigner::lvgl::theme::active_theme().TEXT_SECONDARY, 0);

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
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // No padding on root container; TopNavBar and content container manage their own spacing.

    // Top navigation bar
    top_nav_bar_ = std::make_unique<TopNavBar>(context_);
    TopNavBarConfig nav_config;
    nav_config.title = params_.type == PSBTDetailType::Input ? "Input Details" : "Output Details";
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

    // Inner content container for rows (no extra padding)
    lv_obj_t* rows_container = lv_obj_create(content_container_);
    lv_obj_set_size(rows_container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_border_width(rows_container, 0, 0);
    lv_obj_set_style_pad_all(rows_container, 0, 0);
    lv_obj_set_flex_flow(rows_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(rows_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_bottom(rows_container, 24, 0);

    // Type row
    create_row(rows_container, kTypeLabel, params_.type == PSBTDetailType::Input ? "Input" : "Output");

    // Index row
    create_row(rows_container, kIndexLabel, std::to_string(params_.index).c_str());

    // Address row (may be long, could truncate)
    create_row(rows_container, kAddressLabel, params_.address.empty() ? "Unknown" : params_.address.c_str());

    // Amount row
    create_row(rows_container, kAmountLabel, params_.amount.c_str());

    // Derivation path row (optional)
    if (params_.derivation_path) {
        create_row(rows_container, kDerivationLabel, params_.derivation_path->c_str());
    }

    // Pubkey row (optional)
    if (params_.pubkey) {
        // Truncate pubkey for display
        std::string pubkey_display = *params_.pubkey;
        if (pubkey_display.length() > 16) {
            pubkey_display = pubkey_display.substr(0, 8) + "..." + pubkey_display.substr(pubkey_display.length() - 8);
        }
        create_row(rows_container, kPubkeyLabel, pubkey_display.c_str());
    }

    // Network row
    create_row(rows_container, kNetworkLabel, params_.network.c_str());

    // Footer hint
    footer_label_ = lv_label_create(content_container_);
    lv_label_set_text(footer_label_, "Press RIGHT to view QR (placeholder), BACK to return");
    lv_obj_set_style_text_color(footer_label_, seedsigner::lvgl::theme::active_theme().TEXT_SECONDARY, 0);
    lv_obj_set_style_text_align(footer_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(footer_label_, lv_pct(100));
    lv_obj_set_style_pad_top(footer_label_, 16, 0);
}

void PSBTDetailScreen::destroy() {
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

bool PSBTDetailScreen::handle_input(const InputEvent& input) {
    // First give the top nav bar a chance to handle the input (e.g., hardware back)
    if (top_nav_bar_ && top_nav_bar_->handle_input(input)) {
        return true;
    }
    switch (input.key) {
    case InputKey::Back:
        emit_back();
        return true;
    case InputKey::Right:
        emit_view_qr("address"); // placeholder target
        return true;
    case InputKey::Press:
        // With right_as_select profile, Press is the remapped Right key.
        emit_view_qr("address");
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