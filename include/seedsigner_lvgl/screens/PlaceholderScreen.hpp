#pragma once

#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class PlaceholderScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;

private:
    lv_obj_t* container_{nullptr};
};

}  // namespace seedsigner::lvgl
