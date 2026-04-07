#include "seedsigner_lvgl/components/TopNavBar.hpp"

#include <lvgl.h>
#include <algorithm>

namespace seedsigner::lvgl {

namespace {

constexpr const char* kComponentId = "top_nav_bar";
constexpr const char* kBackAction = "back_requested";
constexpr const char* kHomeAction = "home_requested";
constexpr const char* kCancelAction = "cancel_requested";
constexpr const char* kCustomAction = "action_invoked";

constexpr lv_coord_t kDefaultHeight = 48;
constexpr lv_coord_t kButtonWidth = 48;
constexpr lv_coord_t kButtonHeight = 48;
constexpr lv_coord_t kPadding = 8;
constexpr lv_coord_t kTitleFontSize = 18;
constexpr lv_coord_t kButtonFontSize = 24;

lv_color_t kBackgroundColor = lv_color_hex(0x1a1a1a);
lv_color_t kTextColor = lv_color_hex(0xffffff);
lv_color_t kButtonColor = lv_color_hex(0x333333);
lv_color_t kButtonPressedColor = lv_color_hex(0x555555);
lv_color_t kBorderColor = lv_color_hex(0x444444);

} // anonymous namespace

TopNavBar::TopNavBar(ScreenContext context)
    : context_(std::move(context)) {}

TopNavBar::~TopNavBar() {
    detach();
}

void TopNavBar::attach(lv_obj_t* parent) {
    if (container_ != nullptr) {
        return;
    }
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, lv_pct(100), kDefaultHeight);
    lv_obj_set_style_bg_color(container_, kBackgroundColor, 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, kPadding, 0);
    lv_obj_set_style_pad_row(container_, kPadding, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // Align at top of parent (parent should be column layout)
    lv_obj_align(container_, LV_ALIGN_TOP_LEFT, 0, 0);
    create_widgets();
}

void TopNavBar::detach() {
    destroy_widgets();
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
}

void TopNavBar::set_config(const TopNavBarConfig& config) {
    if (config_ == config) {
        return;
    }
    config_ = config;
    if (container_ != nullptr) {
        destroy_widgets();
        create_widgets();
    }
}

bool TopNavBar::handle_input(const InputEvent& input) {
    // Optionally map hardware keys to bar actions
    // For simplicity, we'll let screens handle hardware mapping.
    // But we can map InputKey::Back to back_requested if show_back is true.
    if (input.key == InputKey::Back && config_.show_back) {
        emit_back();
        return true;
    }
    // Could map InputKey::Home (if exists) to home_requested
    // InputKey::Cancel (if exists) to cancel_requested
    return false;
}

void TopNavBar::emit_back() const {
    context_.emit_action(kBackAction, kComponentId);
}

void TopNavBar::emit_home() const {
    context_.emit_action(kHomeAction, kComponentId);
}

void TopNavBar::emit_cancel() const {
    context_.emit_cancel(kComponentId);
}

void TopNavBar::emit_action(const std::string& action_id) const {
    context_.emit_action(kCustomAction, kComponentId, EventValue{action_id});
}

void TopNavBar::create_widgets() {
    if (container_ == nullptr) {
        return;
    }

    // Left side: back, home, cancel buttons
    if (config_.show_back) {
        back_btn_ = lv_btn_create(container_);
        lv_obj_set_size(back_btn_, kButtonWidth, kButtonHeight);
        lv_obj_set_style_radius(back_btn_, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(back_btn_, kButtonColor, 0);
        lv_obj_set_style_bg_color(back_btn_, kButtonPressedColor, LV_STATE_PRESSED);
        lv_obj_add_event_cb(back_btn_, &TopNavBar::on_back_clicked, LV_EVENT_CLICKED, this);
        lv_obj_t* label = lv_label_create(back_btn_);
        lv_label_set_text(label, LV_SYMBOL_LEFT);
        lv_obj_center(label);
    }

    if (config_.show_home) {
        home_btn_ = lv_btn_create(container_);
        lv_obj_set_size(home_btn_, kButtonWidth, kButtonHeight);
        lv_obj_set_style_radius(home_btn_, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(home_btn_, kButtonColor, 0);
        lv_obj_set_style_bg_color(home_btn_, kButtonPressedColor, LV_STATE_PRESSED);
        lv_obj_add_event_cb(home_btn_, &TopNavBar::on_home_clicked, LV_EVENT_CLICKED, this);
        lv_obj_t* label = lv_label_create(home_btn_);
        lv_label_set_text(label, LV_SYMBOL_HOME);
        lv_obj_center(label);
        // If back also exists, add spacing via pad column (flex gap already set)
    }

    if (config_.show_cancel) {
        cancel_btn_ = lv_btn_create(container_);
        lv_obj_set_size(cancel_btn_, kButtonWidth, kButtonHeight);
        lv_obj_set_style_radius(cancel_btn_, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(cancel_btn_, kButtonColor, 0);
        lv_obj_set_style_bg_color(cancel_btn_, kButtonPressedColor, LV_STATE_PRESSED);
        lv_obj_add_event_cb(cancel_btn_, &TopNavBar::on_cancel_clicked, LV_EVENT_CLICKED, this);
        lv_obj_t* label = lv_label_create(cancel_btn_);
        lv_label_set_text(label, LV_SYMBOL_CLOSE);
        lv_obj_center(label);
    }

    // Center title
    title_label_ = lv_label_create(container_);
    lv_label_set_text(title_label_, config_.title.c_str());
    lv_obj_set_style_text_color(title_label_, kTextColor, 0);
    lv_obj_set_style_text_font(title_label_, &lv_font_montserrat_14, 0);
    lv_obj_set_flex_grow(title_label_, 1);
    lv_obj_set_style_text_align(title_label_, LV_TEXT_ALIGN_CENTER, 0);

    // Right side: custom action buttons
    for (const auto& action : config_.actions) {
        lv_obj_t* btn = lv_btn_create(container_);
        lv_obj_set_size(btn, LV_SIZE_CONTENT, kButtonHeight);
        lv_obj_set_style_min_width(btn, kButtonWidth, 0);
        lv_obj_set_style_radius(btn, 8, 0);
        lv_obj_set_style_bg_color(btn, kButtonColor, 0);
        lv_obj_set_style_bg_color(btn, kButtonPressedColor, LV_STATE_PRESSED);
        if (!action.enabled) {
            lv_obj_add_state(btn, LV_STATE_DISABLED);
        }
        // No extra margin, flex gap already
        lv_obj_set_user_data(btn, const_cast<void*>(static_cast<const void*>(&action.id)));
        lv_obj_add_event_cb(btn, &TopNavBar::on_action_clicked, LV_EVENT_CLICKED, this);
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, action.label.c_str());
        lv_obj_center(label);
        action_buttons_.push_back(btn);
    }

    // Add a spacer if no actions to keep title centered
    if (config_.actions.empty()) {
        lv_obj_t* spacer = lv_obj_create(container_);
        lv_obj_set_size(spacer, kButtonWidth, kButtonHeight);
        lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(spacer, 0, 0);
    }
}

void TopNavBar::destroy_widgets() {
    // LVGL will delete child objects when container is deleted, but we need to clear pointers.
    back_btn_ = nullptr;
    home_btn_ = nullptr;
    cancel_btn_ = nullptr;
    title_label_ = nullptr;
    action_buttons_.clear();
}

void TopNavBar::update_layout() {
    // Not needed for now; widgets are recreated on config change.
}

void TopNavBar::on_back_clicked(lv_event_t* e) {
    auto* bar = static_cast<TopNavBar*>(lv_event_get_user_data(e));
    if (bar != nullptr) {
        bar->emit_back();
    }
}

void TopNavBar::on_home_clicked(lv_event_t* e) {
    auto* bar = static_cast<TopNavBar*>(lv_event_get_user_data(e));
    if (bar != nullptr) {
        bar->emit_home();
    }
}

void TopNavBar::on_cancel_clicked(lv_event_t* e) {
    auto* bar = static_cast<TopNavBar*>(lv_event_get_user_data(e));
    if (bar != nullptr) {
        bar->emit_cancel();
    }
}

void TopNavBar::on_action_clicked(lv_event_t* e) {
    auto* bar = static_cast<TopNavBar*>(lv_event_get_user_data(e));
    if (bar != nullptr) {
        auto* btn = lv_event_get_target(e);
        auto* id_ptr = static_cast<const std::string*>(lv_obj_get_user_data(btn));
        if (id_ptr != nullptr) {
            bar->emit_action(*id_ptr);
        }
    }
}

} // namespace seedsigner::lvgl