#pragma once

#include <memory>
#include <string>

#include "seedsigner_lvgl/components/TopNavBar.hpp"
#include "seedsigner_lvgl/contracts/QRDisplayContract.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class QRDisplayScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;

private:
    void update_brightness_overlay();
    void update_qr_widget();
    void emit_brightness_changed();

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* content_container_{nullptr};
    lv_obj_t* qr_widget_{nullptr};
    lv_obj_t* brightness_overlay_{nullptr};
    QRDisplayParams params_{};
    // Cached QR data to avoid re-rendering on brightness changes
    std::string cached_qr_data_;
    std::unique_ptr<TopNavBar> top_nav_bar_{};
};

}  // namespace seedsigner::lvgl