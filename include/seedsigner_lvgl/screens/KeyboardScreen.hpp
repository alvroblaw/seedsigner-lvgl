#pragma once

#include <memory>
#include <string>
#include <vector>

#include "seedsigner_lvgl/components/TopNavBar.hpp"
#include "seedsigner_lvgl/contracts/KeyboardContract.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class KeyboardScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;

private:
    // UI creation helpers
    void create_text_display();
    void create_keyboard_grid();
    void create_soft_buttons();
    void update_text_display();
    void update_keyboard_grid();
    void update_soft_buttons();

    // Input handling
    void move_selection(int dx, int dy);
    void press_selected_key();
    void press_backspace();
    void press_space();
    void press_cursor_left();
    void press_cursor_right();
    void press_previous_page();
    void press_save();

    // Event emission
    void emit_text_changed();
    void emit_back();
    void emit_save();
    void emit_cursor_left();
    void emit_cursor_right();
    void emit_previous_page();

    // Layout helpers
    void build_layout_map();
    int get_grid_rows() const;
    int get_grid_cols() const;

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* content_container_{nullptr};
    lv_obj_t* text_display_{nullptr}; // lv_textarea or lv_label
    lv_obj_t* keyboard_grid_{nullptr}; // lv_btnmatrix
    lv_obj_t* soft_container_{nullptr}; // container for soft buttons
    lv_obj_t* backspace_btn_{nullptr};
    lv_obj_t* space_btn_{nullptr};
    lv_obj_t* cursor_left_btn_{nullptr};
    lv_obj_t* cursor_right_btn_{nullptr};
    lv_obj_t* previous_page_btn_{nullptr};
    lv_obj_t* save_btn_{nullptr};
    std::unique_ptr<TopNavBar> top_nav_bar_{};

    KeyboardParams params_{};
    std::string current_text_{};
    int cursor_position_{0}; // character index (for future cursor movement)
    int selected_row_{0};
    int selected_col_{0};
    // Cached layout map for btnmatrix (pointers into layout_strings_)
    std::vector<const char*> layout_map_;
    // Null-terminated map for lv_btnmatrix (must outlive the widget)
    std::vector<const char*> layout_map_null_;
    // Storage for button label strings
    std::vector<std::string> layout_strings_;
    // Cached btnmatrix button count
    int btn_count_{0};
};

}  // namespace seedsigner::lvgl