#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "seedsigner_lvgl/contracts/RouteId.hpp"

namespace seedsigner::lvgl {

using ScreenToken = std::uint32_t;
using PropertyMap = std::unordered_map<std::string, std::string>;

struct RouteDescriptor {
    RouteId route_id;
    PropertyMap args;
};

struct ActiveRoute {
    RouteId route_id;
    ScreenToken screen_token{0};
    std::size_t stack_depth{0};
};

}  // namespace seedsigner::lvgl
