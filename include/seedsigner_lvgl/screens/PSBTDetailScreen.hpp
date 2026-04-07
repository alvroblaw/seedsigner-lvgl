#pragma once

#include <string>

#include "seedsigner_lvgl/contracts/PSBTDetailContract.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class PSBTDetailScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;

private:
    void create_layout();
    void emit_back();
    void emit_view_qr(const std::string& target);

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* title_label_{nullptr};
    lv_obj_t* content_container_{nullptr};
    lv_obj_t* footer_label_{nullptr};
    PSBTDetailParams params_{};
};

}  // namespace seedsigner::lvgl