#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "seedsigner_lvgl/components/TopNavBar.hpp"
#include "seedsigner_lvgl/contracts/SettingsContract.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class SettingsMenuScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;
    bool set_data(const PropertyMap& data) override;

    std::size_t item_count() const noexcept { return definitions_.size(); }
    std::size_t selected_index() const noexcept { return selected_index_; }
    const std::string& title() const noexcept { return title_; }

private:
    static std::vector<SettingDefinition> parse_definitions(const PropertyMap& args);
    static std::size_t parse_selected_index(const PropertyMap& args);
    static std::string value_or(const PropertyMap& values, const char* key, const char* fallback = "");

    void apply_selection(std::size_t index);
    void emit_focus_changed(const ScreenContext& context, std::size_t index) const;
    void emit_setting_selected(const ScreenContext& context, std::size_t index) const;
    const SettingDefinition* find_definition(const lv_obj_t* button, std::size_t* index = nullptr) const noexcept;
    static void on_item_event(lv_event_t* event);

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* content_container_{nullptr};
    lv_obj_t* list_{nullptr};
    lv_obj_t* empty_state_{nullptr};
    lv_obj_t* help_label_{nullptr};
    lv_obj_t* footer_label_{nullptr};
    lv_style_t selected_row_style_{};
    lv_style_t row_style_{};
    bool styles_initialized_{false};
    std::string title_;
    std::string help_text_;
    std::string footer_text_;
    std::vector<SettingDefinition> definitions_{};
    std::vector<lv_obj_t*> item_buttons_{};
    std::vector<lv_obj_t*> item_primary_labels_{};
    std::vector<lv_obj_t*> item_secondary_labels_{};
    std::vector<lv_obj_t*> item_accessory_labels_{};
    std::size_t selected_index_{0};
    std::unique_ptr<TopNavBar> top_nav_bar_{};
};

}  // namespace seedsigner::lvgl