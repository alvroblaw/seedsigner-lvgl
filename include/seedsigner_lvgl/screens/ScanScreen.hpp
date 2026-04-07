#pragma once

#include <string>
#include <vector>

#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class ScanScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;
    bool push_frame(const CameraFrame& frame) override;

private:
    void update_progress(unsigned int percent);
    void update_frame_status(bool valid);
    void emit_scan_complete(const std::string& qr_data);
    void emit_scan_cancelled();
    void refresh_ui();

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* preview_img_{nullptr};
    lv_obj_t* instruction_label_{nullptr};
    lv_obj_t* progress_bar_{nullptr};
    lv_obj_t* frame_status_dot_{nullptr};
    lv_obj_t* progress_label_{nullptr}; // optional label for percentage
    
    std::string instruction_text_{"Scan QR code"};
    std::string scan_mode_{"any"};
    unsigned int progress_percent_{0};
    bool frame_valid_{false};
    bool scan_complete_{false};
    std::vector<uint8_t> latest_frame_{};
    uint32_t frame_width_{0};
    uint32_t frame_height_{0};
    uint64_t frame_sequence_{0};
    
    // Mock detection state
    unsigned int mock_frames_received_{0};
    static constexpr unsigned int MOCK_FRAMES_TO_DETECTION = 10;
};

}  // namespace seedsigner::lvgl