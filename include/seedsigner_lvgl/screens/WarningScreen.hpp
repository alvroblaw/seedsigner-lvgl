#pragma once

#include <memory>
#include <string>

#include "seedsigner_lvgl/components/TopNavBar.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

enum class WarningSeverity {
    Warning,
    Error,
    DireWarning,
};

class WarningScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;

private:
    static WarningSeverity parse_severity(const std::string& severity_str);
    static const lv_img_dsc_t* severity_to_icon(WarningSeverity severity);
    static lv_color_t severity_to_title_color(WarningSeverity severity);
    static const char* default_button_text(WarningSeverity severity);

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* content_container_{nullptr};
    lv_obj_t* icon_obj_{nullptr};
    lv_obj_t* body_label_{nullptr};
    lv_obj_t* button_{nullptr};
    lv_obj_t* button_label_{nullptr};
    std::unique_ptr<TopNavBar> top_nav_bar_{};
    WarningSeverity severity_{WarningSeverity::Warning};
    std::string title_;
    std::string body_;
    std::string button_text_;
};

}  // namespace seedsigner::lvgl