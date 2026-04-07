#pragma once

#include <string>

#include "seedsigner_lvgl/contracts/PSBTOverviewContract.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class PSBTOverviewScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;

private:
    void create_layout();
    void update_amount_display();
    void update_diagram();
    void emit_next();
    void emit_back();
    void emit_detail(const std::string& target);

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* title_label_{nullptr};
    lv_obj_t* amount_label_{nullptr};
    lv_obj_t* diagram_container_{nullptr};
    lv_obj_t* inputs_row_{nullptr};
    lv_obj_t* recipients_row_{nullptr};
    lv_obj_t* change_row_{nullptr};
    lv_obj_t* fee_row_{nullptr};
    lv_obj_t* op_return_row_{nullptr};
    lv_obj_t* footer_label_{nullptr};
    PSBTOverviewParams params_{};
};

}  // namespace seedsigner::lvgl