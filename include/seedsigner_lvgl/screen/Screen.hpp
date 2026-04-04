#pragma once

#include <lvgl.h>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

struct ScreenContext {
    lv_obj_t* root{nullptr};
};

class Screen {
public:
    virtual ~Screen() = default;

    virtual void create(const ScreenContext& context, const RouteDescriptor& route) = 0;
    virtual void on_activate() {}
    virtual void on_deactivate() {}
    virtual void destroy() {}
};

}  // namespace seedsigner::lvgl
