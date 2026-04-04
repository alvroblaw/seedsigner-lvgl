#pragma once

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

class Screen {
public:
    virtual ~Screen() = default;

    virtual void create(const RouteDescriptor& route) = 0;
    virtual void on_activate() {}
    virtual void on_deactivate() {}
    virtual void destroy() {}
};

}  // namespace seedsigner::lvgl
