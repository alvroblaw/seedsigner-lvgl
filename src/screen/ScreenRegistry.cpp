#include "seedsigner_lvgl/screen/ScreenRegistry.hpp"

namespace seedsigner::lvgl {

bool ScreenRegistry::register_route(RouteId route_id, Factory factory) {
    if (route_id.empty() || !factory) {
        return false;
    }

    return factories_.emplace(std::move(route_id), std::move(factory)).second;
}

bool ScreenRegistry::has_route(const RouteId& route_id) const {
    return factories_.find(route_id) != factories_.end();
}

std::unique_ptr<Screen> ScreenRegistry::create(const RouteId& route_id) const {
    const auto it = factories_.find(route_id);
    if (it == factories_.end()) {
        return nullptr;
    }

    return it->second();
}

}  // namespace seedsigner::lvgl
