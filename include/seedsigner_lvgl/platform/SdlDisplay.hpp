#pragma once
// SdlDisplay — SDL2-backed display adapter for host desktop builds.
// Renders the LVGL framebuffer into an SDL2 window and exposes a poll_events()
// method that maps keyboard input to seedsigner::lvgl::InputEvent values.
//
// This adapter follows the same pattern as HeadlessDisplay (lv_disp_drv_t
// registration with a flat framebuffer) but adds an SDL2 window for visual
// output and keyboard polling.  No external code was directly adapted; the
// approach is idiomatic LVGL+SDL2 integration (see LVGL docs on "Add a
// display").  License: same as the parent project.

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <lvgl.h>

struct SDL_Window;
struct SDL_Renderer;
union SDL_Event;

namespace seedsigner::lvgl {

struct InputEvent;  // forward — full def in seedsigner_lvgl/runtime/InputEvent.hpp

/// Optional callback invoked when the user requests a quit (window close /
/// Escape on the root screen).  Return true to confirm quit.
using QuitCallback = std::function<bool()>;

class SdlDisplay {
public:
    /// Construct an SDL window of the given LVGL resolution.
    /// @param pixel_scale  Each LVGL pixel is rendered as a pixel_scale×pixel_scale
    ///                     square in the host window (default 2×).
    explicit SdlDisplay(std::uint32_t width, std::uint32_t height, std::uint32_t pixel_scale = 2);
    ~SdlDisplay();

    SdlDisplay(const SdlDisplay&) = delete;
    SdlDisplay& operator=(const SdlDisplay&) = delete;

    /// Poll SDL events for up to @p timeout_ms.  Returns the first mapped
    /// InputEvent, or std::nullopt if no input arrived.
    std::optional<InputEvent> poll_input(std::uint32_t timeout_ms = 16);

    /// Drive the LVGL refresh cycle: call lv_timer_handler() + blit the
    /// framebuffer to the SDL window.
    void refresh();

    /// Direct access to the raw framebuffer (for debugging / screenshots).
    const std::vector<lv_color_t>& framebuffer() const noexcept { return framebuffer_; }

    std::uint32_t width()  const noexcept { return width_; }
    std::uint32_t height() const noexcept { return height_; }
    std::uint32_t flush_count() const noexcept { return flush_count_; }

    /// Install a callback that fires when the user closes the window or hits
    /// Escape with no screen to go back to.
    void set_quit_callback(QuitCallback cb);

    /// Returns true after the SDL window has been closed.
    bool should_quit() const noexcept { return quit_requested_; }

private:
    static void flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);
    void blit_framebuffer();
    std::optional<InputEvent> map_sdl_event(const SDL_Event& ev);

    std::uint32_t width_;
    std::uint32_t height_;
    std::uint32_t pixel_scale_;
    std::uint32_t flush_count_{0};

    lv_disp_draw_buf_t draw_buffer_{};
    lv_disp_drv_t display_driver_{};
    std::vector<lv_color_t> framebuffer_;

    // SDL state — opaque pointers; lifetime managed in .cpp via RAII.
    struct SdlState;
    std::unique_ptr<SdlState> sdl_;

    QuitCallback quit_callback_;
    bool quit_requested_{false};
};

}  // namespace seedsigner::lvgl
