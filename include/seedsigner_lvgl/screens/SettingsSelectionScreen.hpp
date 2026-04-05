#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "seedsigner_lvgl/contracts/SettingsContract.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class SettingsSelectionScreen : public Screen {
public:
    using Item = SettingItemDefinition;

    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;
    bool set_data(const PropertyMap& data) override;

    std::size_t item_count() const noexcept { return items_.size(); }
    std::size_t selected_index() const noexcept { return selected_index_; }
    const std::string& title() const noexcept { return title_; }
    const std::string& subtitle() const noexcept { return subtitle_; }
    const std::string& section_title() const noexcept { return section_title_; }

private:
    static std::size_t parse_selected_index(const PropertyMap& args);
    static std::string value_or(const PropertyMap& values, const char* key, const char* fallback = "");

    void apply_selection(std::size_t index);
    void emit_focus_changed(const ScreenContext& context, std::size_t index) const;
    void emit_item_selected(const ScreenContext& context, std::size_t index) const;
    void apply_current_values_from_route(const PropertyMap& args);
    void toggle_current_value(std::size_t index);
    bool is_current_value(std::string_view id) const noexcept;
    std::vector<std::string> current_value_ids() const;
    std::string payload_for_item(std::size_t index, std::string_view event_name) const;
    void refresh_item_accessories();
    const char* accessory_text_for_item(const Item& item) const;
    const Item* find_item(const lv_obj_t* button, std::size_t* index = nullptr) const noexcept;
    static void on_item_event(lv_event_t* event);

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* list_{nullptr};
    lv_obj_t* empty_state_{nullptr};
    lv_obj_t* help_label_{nullptr};
    lv_obj_t* footer_label_{nullptr};
    lv_style_t selected_row_style_{};
    lv_style_t row_style_{};
    bool styles_initialized_{false};
    std::string title_;
    std::string subtitle_;
    std::string section_title_;
    std::string help_text_;
    std::string footer_text_;
    SettingDefinition definition_{};
    std::string current_value_id_;
    std::unordered_set<std::string> current_value_ids_set_{};
    std::vector<Item> items_{};
    std::vector<lv_obj_t*> item_buttons_{};
    std::vector<lv_obj_t*> item_accessory_labels_{};
    std::size_t selected_index_{0};
};

}  // namespace seedsigner::lvgl
