#include "seedsigner_lvgl/screens/ScreensaverScreen.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"

#include <lvgl.h>

namespace seedsigner::lvgl {

namespace {

constexpr const char* kScreensaverComponent = "screensaver";
constexpr const char* kDismissedAction = "screensaver_dismissed";

constexpr int kDefaultLogoSize = 80;
constexpr int kSlideRange = 100; // pixels to slide
constexpr int kFadePeriod = 3000; // ms for full fade cycle

lv_obj_t* create_animation_object(lv_obj_t* parent, const ScreensaverParams& params) {
    lv_obj_t* obj = nullptr;
    if (params.image_path) {
        // Load image from file (placeholder)
        obj = lv_img_create(parent);
        lv_img_set_src(obj, LV_SYMBOL_IMAGE);
        lv_obj_set_size(obj, kDefaultLogoSize, kDefaultLogoSize);
    } else {
        // Default: a simple circle
        obj = lv_obj_create(parent);
        lv_obj_set_size(obj, kDefaultLogoSize, kDefaultLogoSize);
        lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(obj, seedsigner::lvgl::theme::colors::PRIMARY, 0);
        lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(obj, 0, 0);
    }
    return obj;
}

}  // namespace

void ScreensaverScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    params_ = parse_screensaver_params(route.args);
    dismissed_ = false;

    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container_, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);

    // Animation object
    animation_obj_ = create_animation_object(container_, params_);
    lv_obj_center(animation_obj_);

    // Wake‑up overlay (initially hidden, shown after a short delay or on first touch)
    if (params_.show_wakeup_overlay) {
        wakeup_overlay_ = lv_obj_create(container_);
        lv_obj_set_size(wakeup_overlay_, lv_pct(100), lv_pct(100));
        lv_obj_set_style_bg_color(wakeup_overlay_, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(wakeup_overlay_, LV_OPA_50, 0);
        lv_obj_set_style_pad_all(wakeup_overlay_, 0, 0);
        lv_obj_add_flag(wakeup_overlay_, LV_OBJ_FLAG_HIDDEN);

        lv_obj_t* label = lv_label_create(wakeup_overlay_);
        lv_label_set_text(label, "Touch to continue");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_center(label);
    }

    // Animation timer (if animation type specified)
    if (params_.animation_type) {
        animation_timer_ = lv_timer_create([](lv_timer_t* timer) {
            auto* self = static_cast<ScreensaverScreen*>(timer->user_data);
            self->update_animation();
        }, params_.update_interval_ms, this);
    }
}

void ScreensaverScreen::destroy() {
    if (animation_timer_) {
        lv_timer_del(animation_timer_);
        animation_timer_ = nullptr;
    }
    if (container_) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
    animation_obj_ = nullptr;
    wakeup_overlay_ = nullptr;
}

bool ScreensaverScreen::handle_input(const InputEvent& input) {
    if (dismissed_) {
        return false;
    }
    if (input.key == InputKey::Press ||
        input.key == InputKey::Back) {
        dismiss();
        return true;
    }
    return false;
}

void ScreensaverScreen::on_activate() {
    // Nothing special needed
}

void ScreensaverScreen::on_deactivate() {
    // Nothing special needed
}

void ScreensaverScreen::update_animation() {
    if (!animation_obj_ || dismissed_) {
        return;
    }
    const std::string& anim_type = params_.animation_type.value_or("");
    if (anim_type == "slide") {
        // Simple horizontal slide back and forth
        static int dx = 0;
        static int dir = 1;
        dx += dir;
        if (dx > kSlideRange) dir = -1;
        if (dx < -kSlideRange) dir = 1;
        lv_obj_set_x(animation_obj_, dx);
    } else if (anim_type == "fade") {
        // Fade in/out
        static lv_opa_t opa = LV_OPA_0;
        static int dir = 1;
        opa += dir * 5;
        if (opa >= LV_OPA_COVER) dir = -1;
        if (opa <= LV_OPA_0) dir = 1;
        lv_obj_set_style_bg_opa(animation_obj_, opa, 0);
    }
    // else no animation
}

void ScreensaverScreen::dismiss() {
    if (dismissed_) {
        return;
    }
    dismissed_ = true;
    if (animation_timer_) {
        lv_timer_del(animation_timer_);
        animation_timer_ = nullptr;
    }
    context_.emit_action(kDismissedAction, kScreensaverComponent);
}

}  // namespace seedsigner::lvgl