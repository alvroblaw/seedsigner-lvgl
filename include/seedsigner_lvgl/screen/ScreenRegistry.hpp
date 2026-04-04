#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include "seedsigner_lvgl/contracts/RouteId.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class ScreenRegistry {
public:
    using Factory = std::function<std::unique_ptr<Screen>()>;

    bool register_route(RouteId route_id, Factory factory);
    bool has_route(const RouteId& route_id) const;
    std::unique_ptr<Screen> create(const RouteId& route_id) const;

private:
    std::unordered_map<RouteId, Factory, RouteIdHash> factories_;
};

}  // namespace seedsigner::lvgl
