#include "seedsigner_lvgl/components/TopNavBar.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"
#include "icons.h"

#include <cstdio>
#include <lvgl.h>
#include <algorithm>

namespace seedsigner::lvgl {

namespace {

constexpr const char* kComponentId = "top_nav_bar";
constexpr const char* kBackAction = "back_requested";
constexpr const char* kHomeAction = "home_requested";
constexpr const char* kCancelAction = "cancel_requested";
constexpr const char* kCustomAction = "action_invoked";

constexpr lv_coord_t kDefaultHeight = theme::spacing::TOPBAR_HEIGHT;
constexpr lv_coord_t kButtonWidth = theme::spacing::TOPBAR_BUTTON_SIZE;
constexpr lv_coord_t kButtonHeight = theme::spacing::TOPBAR_BUTTON_SIZE;
constexpr lv_coord_t kPadding = theme::spacing::COMPONENT_PADDING;
constexpr lv_coord_t kTitleFontSize = 18; // Overridden by theme font
constexpr lv_coord_t kButtonFontSize = 24;

// Colors are now defined in SeedSignerTheme.hpp

} // anonymous namespace

TopNavBar::TopNavBar(ScreenContext context)
    : context_(std::move(context)) {}

TopNavBar::~TopNavBar() {
    fprintf(stderr, "[TopNavBar] destructor container_=%p\n", container_); fflush(stderr);
    detach();
}

void TopNavBar::attach(lv_obj_t* parent) {
    if (container_ != nullptr) {
        return;
    }
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, lv_pct(100), kDefaultHeight);
    seedsigner::lvgl::theme::apply_topbar_style(container_);
    lv_obj_set_style_pad_row(container_, kPadding, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // Align at top of parent (parent should be column layout)
    lv_obj_align(container_, LV_ALIGN_TOP_LEFT, 0, 0);
    create_widgets();
}

void TopNavBar::detach() {
    fprintf(stderr, "[TopNavBar] detach container_=%p parent=%p\n", container_, container_ ? lv_obj_get_parent(container_) : nullptr); fflush(stderr);
    destroy_widgets();
    // Do NOT delete container_ here; the parent (screen root container) will delete it.
    // Simply clear the pointer to avoid dangling reference.
    container_ = nullptr;
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
        seedsigner::lvgl::theme::apply_button_style(back_btn_, false);
        lv_obj_set_style_radius(back_btn_, LV_RADIUS_CIRCLE, 0); // Keep circular shape
        lv_obj_add_event_cb(back_btn_, &TopNavBar::on_back_clicked, LV_EVENT_CLICKED, this);
        lv_obj_t* img = lv_img_create(back_btn_);
        lv_img_set_src(img, &img_back);
        lv_obj_center(img);
        lv_obj_set_style_img_recolor(img, seedsigner::lvgl::theme::active_theme().TEXT_PRIMARY, 0);
        lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
        lv_obj_set_style_img_recolor(img, seedsigner::lvgl::theme::active_theme().PRIMARY, LV_STATE_PRESSED);
        lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_obj_set_style_opa(img, LV_OPA_40, LV_STATE_DISABLED);
    }

    if (config_.show_home) {
        home_btn_ = lv_btn_create(container_);
        lv_obj_set_size(home_btn_, kButtonWidth, kButtonHeight);
        seedsigner::lvgl::theme::apply_button_style(home_btn_, false);
        lv_obj_set_style_radius(home_btn_, LV_RADIUS_CIRCLE, 0); // Keep circular shape
        lv_obj_add_event_cb(home_btn_, &TopNavBar::on_home_clicked, LV_EVENT_CLICKED, this);
        lv_obj_t* img = lv_img_create(home_btn_);
        lv_img_set_src(img, &img_home);
        lv_obj_center(img);
        lv_obj_set_style_img_recolor(img, seedsigner::lvgl::theme::active_theme().TEXT_PRIMARY, 0);
        lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
        lv_obj_set_style_img_recolor(img, seedsigner::lvgl::theme::active_theme().PRIMARY, LV_STATE_PRESSED);
        lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_obj_set_style_opa(img, LV_OPA_40, LV_STATE_DISABLED);
        // If back also exists, add spacing via pad column (flex gap already set)
    }

    if (config_.show_cancel) {
        cancel_btn_ = lv_btn_create(container_);
        lv_obj_set_size(cancel_btn_, kButtonWidth, kButtonHeight);
        seedsigner::lvgl::theme::apply_button_style(cancel_btn_, false);
        lv_obj_set_style_radius(cancel_btn_, LV_RADIUS_CIRCLE, 0); // Keep circular shape
        lv_obj_add_event_cb(cancel_btn_, &TopNavBar::on_cancel_clicked, LV_EVENT_CLICKED, this);
        lv_obj_t* img = lv_img_create(cancel_btn_);
        lv_img_set_src(img, &img_close);
        lv_obj_center(img);
        lv_obj_set_style_img_recolor(img, seedsigner::lvgl::theme::active_theme().TEXT_PRIMARY, 0);
        lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
        lv_obj_set_style_img_recolor(img, seedsigner::lvgl::theme::active_theme().PRIMARY, LV_STATE_PRESSED);
        lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_obj_set_style_opa(img, LV_OPA_40, LV_STATE_DISABLED);
    }

    // Center title
    title_label_ = lv_label_create(container_);
    lv_label_set_text(title_label_, config_.title.c_str());
    lv_obj_set_style_text_color(title_label_, seedsigner::lvgl::theme::active_theme().TEXT_PRIMARY, 0);
    lv_obj_set_style_text_font(title_label_, seedsigner::lvgl::theme::typography::TITLE, 0);
    lv_obj_set_flex_grow(title_label_, 1);
    lv_obj_set_style_text_align(title_label_, LV_TEXT_ALIGN_CENTER, 0);

    // Right side: custom action buttons
    for (const auto& action : config_.actions) {
        lv_obj_t* btn = lv_btn_create(container_);
        lv_obj_set_size(btn, LV_SIZE_CONTENT, kButtonHeight);
        lv_obj_set_style_min_width(btn, kButtonWidth, 0);
        seedsigner::lvgl::theme::apply_button_style(btn, false);
        if (!action.enabled) {
            lv_obj_add_state(btn, LV_STATE_DISABLED);
        }
        // No extra margin, flex gap already
        lv_obj_set_user_data(btn, const_cast<void*>(static_cast<const void*>(&action.id)));
        lv_obj_add_event_cb(btn, &TopNavBar::on_action_clicked, LV_EVENT_CLICKED, this);
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, action.label.c_str());
        lv_obj_set_style_text_color(label, seedsigner::lvgl::theme::active_theme().TEXT_PRIMARY, 0);
        lv_obj_set_style_text_color(label, seedsigner::lvgl::theme::active_theme().TEXT_DISABLED, LV_STATE_DISABLED);
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