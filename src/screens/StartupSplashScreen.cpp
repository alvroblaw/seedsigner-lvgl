#include "seedsigner_lvgl/screens/StartupSplashScreen.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"
#include "icons.h"

#include <lvgl.h>

namespace seedsigner::lvgl {

namespace {

constexpr const char* kSplashComponent = "startup_splash";
constexpr const char* kCompletedAction = "splash_completed";
constexpr const char* kSkippedAction = "splash_skipped";

lv_obj_t* create_logo(lv_obj_t* parent, const std::optional<std::string>& logo_path) {
    // Use actual SeedSigner logo
    lv_obj_t* img = lv_img_create(parent);
    lv_img_set_src(img, &img_logo_black_240);
    // Scale down if needed (logo is 240x240)
    lv_obj_set_size(img, 160, 160); // reasonable size
    lv_obj_set_style_align(img, LV_ALIGN_CENTER, 0);
    return img;
}

}  // namespace

void StartupSplashScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    params_ = parse_startup_splash_params(route.args);
    completed_ = false;

    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 16, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Logo
    logo_img_ = create_logo(container_, params_.logo_path);
    if (logo_img_) {
        lv_obj_set_style_pad_bottom(logo_img_, 24, 0);
    }

    // Version label
    version_label_ = lv_label_create(container_);
    lv_label_set_text(version_label_, params_.version.c_str());
    lv_obj_set_style_text_font(version_label_, &lv_font_montserrat_14, 0);
    lv_obj_set_style_pad_bottom(version_label_, 32, 0);

    // Timer for auto‑completion (if timeout > 0)
    if (params_.timeout_ms > 0) {
        timer_ = lv_timer_create([](lv_timer_t* timer) {
            auto* self = static_cast<StartupSplashScreen*>(timer->user_data);
            self->emit_completed(false); // auto‑completed
        }, params_.timeout_ms, this);
        lv_timer_set_repeat_count(timer_, 1); // one‑shot
    }
}

void StartupSplashScreen::destroy() {
    if (timer_) {
        lv_timer_del(timer_);
        timer_ = nullptr;
    }
    if (container_) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
    // LVGL objects are deleted with container
    logo_img_ = nullptr;
    version_label_ = nullptr;
}

bool StartupSplashScreen::handle_input(const InputEvent& input) {
    if (completed_) {
        return false;
    }
    if (params_.skip_on_input && (input.key == InputKey::Press ||
                                  input.key == InputKey::Back)) {
        emit_completed(true); // skipped
        return true;
    }
    return false;
}

void StartupSplashScreen::on_activate() {
    // Nothing special needed
}

void StartupSplashScreen::on_deactivate() {
    // Nothing special needed
}

void StartupSplashScreen::emit_completed(bool skipped) {
    if (completed_) {
        return;
    }
    completed_ = true;
    if (timer_) {
        lv_timer_del(timer_);
        timer_ = nullptr;
    }
    if (skipped) {
        context_.emit_action(kSkippedAction, kSplashComponent);
    } else {
        context_.emit_action(kCompletedAction, kSplashComponent);
    }
    // The host should navigate away; we don't destroy screen here.
}

}  // namespace seedsigner::lvgl