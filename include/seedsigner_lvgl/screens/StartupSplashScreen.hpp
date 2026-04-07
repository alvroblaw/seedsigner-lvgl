#pragma once

#include "seedsigner_lvgl/contracts/StartupSplashContract.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class StartupSplashScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;
    void on_activate() override;
    void on_deactivate() override;

private:
    void on_timer_complete();
    void emit_completed(bool skipped);

    ScreenContext context_{};
    StartupSplashParams params_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* logo_img_{nullptr};
    lv_obj_t* version_label_{nullptr};
    lv_obj_t* progress_bar_{nullptr};
    lv_timer_t* timer_{nullptr};
    bool completed_{false};
};

}  // namespace seedsigner::lvgl