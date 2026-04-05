#pragma once

#include <string>
#include <vector>

#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class CameraPreviewScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;
    bool set_data(const PropertyMap& data) override;
    bool patch_data(const PropertyMap& patch) override;
    bool push_frame(const CameraFrame& frame) override;

private:
    void apply_data(const PropertyMap& data, bool replace);
    void refresh_preview();
    void refresh_labels();

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* title_label_{nullptr};
    lv_obj_t* preview_panel_{nullptr};
    lv_obj_t* preview_canvas_{nullptr};
    lv_obj_t* frame_label_{nullptr};
    lv_obj_t* status_label_{nullptr};
    std::vector<lv_color_t> preview_canvas_buffer_{};
    std::vector<std::uint8_t> latest_frame_{};
    std::string title_{"Scan QR"};
    std::string status_{"Waiting for external frames"};
    std::uint64_t frame_sequence_{0};
    std::uint32_t frame_width_{0};
    std::uint32_t frame_height_{0};
    std::uint32_t frame_stride_{0};
    std::size_t frame_bytes_{0};
};

}  // namespace seedsigner::lvgl
