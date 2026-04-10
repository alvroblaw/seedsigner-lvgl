#pragma once

#include <memory>
#include <string>

#include "seedsigner_lvgl/components/TopNavBar.hpp"
#include "seedsigner_lvgl/contracts/PSBTMathContract.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class PSBTMathScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;

private:
    void create_layout();
    void update_table();
    void emit_next();
    void emit_back();
    void emit_detail(const std::string& target);

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* content_container_{nullptr};
    lv_obj_t* table_{nullptr};
    lv_obj_t* footer_label_{nullptr};
    PSBTMathParams params_{};
    std::unique_ptr<TopNavBar> top_nav_bar_{};
};

}  // namespace seedsigner::lvgl