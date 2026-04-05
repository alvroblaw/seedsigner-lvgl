#pragma once

#include <string>

#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class ResultScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;
    bool set_data(const PropertyMap& data) override;
    bool patch_data(const PropertyMap& patch) override;

private:
    void apply_data(const PropertyMap& data, bool replace);
    void refresh_labels();

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* title_label_{nullptr};
    lv_obj_t* body_label_{nullptr};
    std::string title_{"Scan Result"};
    std::string body_{"No result yet."};
    std::string continue_action_{"continue"};
};

}  // namespace seedsigner::lvgl
