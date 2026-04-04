#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/navigation/NavigationController.hpp"
#include "seedsigner_lvgl/platform/HeadlessDisplay.hpp"
#include "seedsigner_lvgl/runtime/EventQueue.hpp"
#include "seedsigner_lvgl/screen/ScreenRegistry.hpp"

namespace seedsigner::lvgl {

struct RuntimeConfig {
    std::uint32_t width{240};
    std::uint32_t height{320};
    std::size_t event_queue_capacity{16};
};

class UiRuntime {
public:
    explicit UiRuntime(RuntimeConfig config = {});
    ~UiRuntime();

    bool init();

    ScreenRegistry& screen_registry() noexcept { return screen_registry_; }
    const ScreenRegistry& screen_registry() const noexcept { return screen_registry_; }

    std::optional<ActiveRoute> activate(const RouteDescriptor& route);
    std::optional<ActiveRoute> replace(const RouteDescriptor& route);
    std::optional<ActiveRoute> get_active_route() const noexcept;

    bool emit(UiEvent event);
    std::optional<UiEvent> next_event();

    void tick(std::uint32_t elapsed_ms);
    void refresh_now();
    const HeadlessDisplay* display() const noexcept { return display_.get(); }

private:
    RuntimeConfig config_;
    bool initialized_{false};
    ScreenRegistry screen_registry_;
    NavigationController navigation_controller_;
    EventQueue event_queue_;
    std::unique_ptr<HeadlessDisplay> display_;
    std::uint64_t now_ms_{0};
};

}  // namespace seedsigner::lvgl
