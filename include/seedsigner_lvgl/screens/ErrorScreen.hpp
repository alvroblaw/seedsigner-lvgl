#pragma once

#include "seedsigner_lvgl/screens/WarningScreen.hpp"

namespace seedsigner::lvgl {

class ErrorScreen : public WarningScreen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
};

}  // namespace seedsigner::lvgl