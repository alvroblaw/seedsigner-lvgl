#pragma once
// SdlDisplay — SDL2-backed display adapter for host desktop builds.
// Uses the LVGL v9 display + indev API (lv_display_create / lv_indev_create).

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <lvgl.h>

struct SDL_Texture;
struct SDL_Window;
struct SDL_Renderer;
union SDL_Event;

namespace seedsigner::lvgl {

struct InputEvent;  // forward — full def in seedsigner_lvgl/runtime/InputEvent.hpp

using QuitCallback = std::function<bool()>;

class SdlDisplay {
public:
    explicit SdlDisplay(std::uint32_t width, std::uint32_t height, std::uint32_t pixel_scale = 2);
    ~SdlDisplay();

    SdlDisplay(const SdlDisplay&) = delete;
    SdlDisplay& operator=(const SdlDisplay&) = delete;

    std::optional<InputEvent> poll_input(std::uint32_t timeout_ms = 16);
    void refresh();

    /// Raw framebuffer bytes — RGB565 pixels (2 bytes each), row-major.
    const std::vector<std::uint8_t>& framebuffer() const noexcept { return framebuffer_; }

    std::uint32_t width()  const noexcept { return width_; }
    std::uint32_t height() const noexcept { return height_; }
    std::uint32_t flush_count() const noexcept { return flush_count_; }

    void set_quit_callback(QuitCallback cb);
    bool should_quit() const noexcept { return quit_requested_; }

    void enable_pointer();
    bool pointer_enabled() const noexcept { return pointer_enabled_; }

    void handle_mouse_event(const SDL_Event& ev);
    std::optional<InputEvent> poll_all_events();

    bool switch_resolution(std::uint32_t new_width, std::uint32_t new_height);

private:
    static void flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map);
    void blit_framebuffer();
    std::optional<InputEvent> map_sdl_event(const SDL_Event& ev);
    void create_sdl_window();

    std::uint32_t width_;
    std::uint32_t height_;
    std::uint32_t pixel_scale_;
    std::uint32_t flush_count_{0};

    lv_display_t* display_{nullptr};
    std::vector<std::uint8_t> draw_buf_;    ///< LVGL render scratch (partial rows)
    std::vector<std::uint8_t> framebuffer_; ///< Full-frame buffer (always coherent)

    // SDL state — opaque pointers; lifetime managed in .cpp via RAII.
    struct SdlState;
    std::unique_ptr<SdlState> sdl_;

    SDL_Texture* texture_{nullptr};
    std::vector<std::uint32_t> argb_buffer_;

    QuitCallback quit_callback_;
    bool quit_requested_{false};

    // --- Pointer (mouse/touch) indev ---
    bool pointer_enabled_{false};
    lv_indev_t* pointer_device_{nullptr};

    lv_point_t pointer_pos_{0, 0};
    bool pointer_pressed_{false};

    static void pointer_read_cb(lv_indev_t* indev, lv_indev_data_t* data);
};

}  // namespace seedsigner::lvgl
