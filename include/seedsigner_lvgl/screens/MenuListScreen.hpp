#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class MenuListScreen : public Screen {
public:
    struct Item {
        std::string id;
        std::string label;
        std::string secondary_text;
        std::string accessory;
    };

    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;
    bool set_data(const PropertyMap& data) override;

    std::size_t item_count() const noexcept { return items_.size(); }
    std::size_t selected_index() const noexcept { return selected_index_; }
    const std::string& title() const noexcept { return title_; }
    lv_obj_t* item_button(std::size_t index) const noexcept;

private:
    static std::vector<Item> parse_items(const PropertyMap& args);
    static std::size_t parse_selected_index(const PropertyMap& args);
    static std::string value_or(const PropertyMap& values, const char* key, const char* fallback);

    void apply_selection(std::size_t index);
    void emit_focus_changed(const ScreenContext& context, std::size_t index) const;
    void emit_item_selected(const ScreenContext& context, std::size_t index) const;
    const Item* find_item(const lv_obj_t* button, std::size_t* index = nullptr) const noexcept;
    static void on_item_event(lv_event_t* event);

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* list_{nullptr};
    lv_obj_t* empty_state_{nullptr};
    lv_style_t selected_row_style_{};
    lv_style_t row_style_{};
    bool styles_initialized_{false};
    std::string title_;
    std::vector<Item> items_{};
    std::vector<lv_obj_t*> item_buttons_{};
    std::vector<lv_obj_t*> item_primary_labels_{};
    std::vector<lv_obj_t*> item_secondary_labels_{};
    std::vector<lv_obj_t*> item_accessory_labels_{};
    std::size_t selected_index_{0};
};

}  // namespace seedsigner::lvgl
