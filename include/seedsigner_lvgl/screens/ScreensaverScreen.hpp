#pragma once

#include "seedsigner_lvgl/contracts/ScreensaverContract.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class ScreensaverScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;
    void on_activate() override;
    void on_deactivate() override;

private:
    void update_animation();
    void dismiss();

    ScreenContext context_{};
    ScreensaverParams params_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* animation_obj_{nullptr}; // could be image or canvas
    lv_obj_t* wakeup_overlay_{nullptr};
    lv_timer_t* animation_timer_{nullptr};
    bool dismissed_{false};
};

}  // namespace seedsigner::lvgl