#pragma once

#include <string>
#include <utility>

namespace seedsigner::lvgl {

class RouteId {
public:
    RouteId() = default;
    explicit RouteId(std::string value) : value_(std::move(value)) {}

    const std::string& value() const noexcept { return value_; }
    bool empty() const noexcept { return value_.empty(); }

    friend bool operator==(const RouteId& lhs, const RouteId& rhs) noexcept {
        return lhs.value_ == rhs.value_;
    }

    friend bool operator!=(const RouteId& lhs, const RouteId& rhs) noexcept {
        return !(lhs == rhs);
    }

private:
    std::string value_;
};

struct RouteIdHash {
    std::size_t operator()(const RouteId& route_id) const noexcept {
        return std::hash<std::string>{}(route_id.value());
    }
};

}  // namespace seedsigner::lvgl
