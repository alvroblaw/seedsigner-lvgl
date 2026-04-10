#include "seedsigner_lvgl/screens/KeyboardScreen.hpp"

#include <lvgl.h>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <iostream>

namespace seedsigner::lvgl {
namespace {

constexpr const char* kKeyboardComponent = "keyboard";
constexpr const char* kTextChangedAction = "text_changed";
constexpr const char* kBackAction = "back";
constexpr const char* kSaveAction = "save";
constexpr const char* kCursorLeftAction = "cursor_left";
constexpr const char* kCursorRightAction = "cursor_right";
constexpr const char* kPreviousPageAction = "previous_page";

// Default grid dimensions
constexpr int kDefaultRows = 4;
constexpr int kDefaultCols = 10;

// Soft button labels
constexpr const char* kBackspaceLabel = "<-";
constexpr const char* kSpaceLabel = "SPACE";
constexpr const char* kCursorLeftLabel = "<";
constexpr const char* kCursorRightLabel = ">";
constexpr const char* kPreviousPageLabel = "<<";
constexpr const char* kSaveLabel = "SAVE";

// Layout strings for lowercase letters a-z
constexpr const char* kLowercaseLetters = "abcdefghijklmnopqrstuvwxyz";
constexpr const char* kUppercaseLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
constexpr const char* kDigits = "0123456789";
constexpr const char* kSymbols = "!@#$%^&*()_+-=[]{}|;:'\",.<>?/`~";

// Helper to split a string into rows of equal length
std::vector<const char*> split_into_rows(const std::string& chars, int cols) {
    std::vector<const char*> rows;
    std::string row;
    for (size_t i = 0; i < chars.size(); ++i) {
        row += chars[i];
        if ((i + 1) % cols == 0 || i == chars.size() - 1) {
            rows.push_back(strdup(row.c_str())); // Note: memory leak, but fine for now
            row.clear();
        }
    }
    return rows;
}

}  // namespace

void KeyboardScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    std::cout << "KeyboardScreen::create entered" << std::endl;
    context_ = context;
    params_ = parse_keyboard_params(route.args);
    std::cout << "layout=" << static_cast<int>(params_.layout) << std::endl;
    if (params_.initial_value) {
        current_text_ = *params_.initial_value;
    }
    cursor_position_ = static_cast<int>(current_text_.size());
    std::cout << "initial text='" << current_text_ << "'" << std::endl;

    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // No padding on root container; TopNavBar and content container manage their own spacing.

    // Top navigation bar
    top_nav_bar_ = std::make_unique<TopNavBar>(context_);
    TopNavBarConfig nav_config;
    nav_config.title = "Enter Text";
    nav_config.show_back = true;
    nav_config.show_home = false;
    nav_config.show_cancel = false;
    // No custom actions
    top_nav_bar_->set_config(nav_config);
    top_nav_bar_->attach(container_);

    // Content container (everything below the nav bar)
    content_container_ = lv_obj_create(container_);
    lv_obj_set_size(content_container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(content_container_, 8, 0);
    lv_obj_set_flex_flow(content_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(content_container_, 1); // Take remaining height

    create_text_display();
    create_keyboard_grid();
    create_soft_buttons();
}

void KeyboardScreen::destroy() {
    // TopNavBar must be cleared before its parent container is deleted.
    top_nav_bar_.reset();
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
    content_container_ = nullptr;
    text_display_ = nullptr;
    keyboard_grid_ = nullptr;
    soft_container_ = nullptr;
    backspace_btn_ = nullptr;
    space_btn_ = nullptr;
    cursor_left_btn_ = nullptr;
    cursor_right_btn_ = nullptr;
    previous_page_btn_ = nullptr;
    save_btn_ = nullptr;
    context_ = {};
    layout_map_.clear();
    layout_strings_.clear();
}

bool KeyboardScreen::handle_input(const InputEvent& input) {
    // First give the top nav bar a chance to handle the input (e.g., hardware back)
    if (top_nav_bar_ && top_nav_bar_->handle_input(input)) {
        return true;
    }
    switch (input.key) {
        case InputKey::Up:
            move_selection(0, -1);
            return true;
        case InputKey::Down:
            move_selection(0, 1);
            return true;
        case InputKey::Left:
            move_selection(-1, 0);
            return true;
        case InputKey::Right:
            move_selection(1, 0);
            return true;
        case InputKey::Press:
            press_selected_key();
            return true;
        case InputKey::Back:
            // If backspace is allowed, maybe treat as backspace? Usually Back button is for exiting screen.
            // We'll emit back event.
            emit_back();
            return true;
        default:
            break;
    }
    return false;
}

// ==================== UI Creation ====================

void KeyboardScreen::create_text_display() {
    std::cout << "KeyboardScreen::create_text_display" << std::endl;
    // Create a text area for display and editing
    text_display_ = lv_textarea_create(content_container_);
    lv_obj_set_width(text_display_, lv_pct(100));
    lv_obj_set_height(text_display_, LV_SIZE_CONTENT);
    lv_textarea_set_text(text_display_, current_text_.c_str());
    lv_textarea_set_cursor_pos(text_display_, cursor_position_);
    lv_textarea_set_one_line(text_display_, true);
    lv_textarea_set_placeholder_text(text_display_, params_.placeholder ? params_.placeholder->c_str() : "");
    // Disable input via LVGL's built‑in keyboard; we handle input ourselves
    lv_textarea_set_text_selection(text_display_, false);
    lv_obj_clear_flag(text_display_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_pad_bottom(text_display_, 16, 0);
}

void KeyboardScreen::create_keyboard_grid() {
    std::cout << "KeyboardScreen::create_keyboard_grid" << std::endl;
    keyboard_grid_ = lv_btnmatrix_create(content_container_);
    lv_obj_set_width(keyboard_grid_, lv_pct(100));
    lv_obj_set_height(keyboard_grid_, LV_SIZE_CONTENT);
    // Build layout map
    build_layout_map();
    std::cout << "btn_count_=" << btn_count_ << std::endl;
    // Convert vector to array of const char* (last element must be NULL)
    std::vector<const char*> map_with_null = layout_map_;
    map_with_null.push_back(nullptr);
    lv_btnmatrix_set_map(keyboard_grid_, map_with_null.data());
    // Set button control: all buttons are checkable (so they can be highlighted), none checked initially
    for (int i = 0; i < btn_count_; ++i) {
        lv_btnmatrix_set_btn_ctrl(keyboard_grid_, i, LV_BTNMATRIX_CTRL_CHECKABLE);
    }
    // Set selected button (first)
    selected_row_ = 0;
    selected_col_ = 0;
    lv_btnmatrix_set_btn_ctrl(keyboard_grid_, 0, LV_BTNMATRIX_CTRL_CHECKED);
    lv_obj_set_style_pad_bottom(keyboard_grid_, 16, 0);
}

void KeyboardScreen::create_soft_buttons() {
    // Container for soft buttons in a horizontal row
    soft_container_ = lv_obj_create(content_container_);
    lv_obj_set_size(soft_container_, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(soft_container_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(soft_container_, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(soft_container_, LV_OBJ_FLAG_SCROLLABLE);

    if (params_.allow_backspace) {
        backspace_btn_ = lv_btn_create(soft_container_);
        lv_obj_t* label = lv_label_create(backspace_btn_);
        lv_label_set_text(label, kBackspaceLabel);
        lv_obj_set_size(backspace_btn_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_add_event_cb(backspace_btn_, [](lv_event_t* e) {
            // Not used; we handle input via handle_input
        }, LV_EVENT_CLICKED, this);
    }
    if (params_.allow_space) {
        space_btn_ = lv_btn_create(soft_container_);
        lv_obj_t* label = lv_label_create(space_btn_);
        lv_label_set_text(label, kSpaceLabel);
        lv_obj_set_size(space_btn_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    }
    if (params_.allow_cursor_left) {
        cursor_left_btn_ = lv_btn_create(soft_container_);
        lv_obj_t* label = lv_label_create(cursor_left_btn_);
        lv_label_set_text(label, kCursorLeftLabel);
        lv_obj_set_size(cursor_left_btn_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    }
    if (params_.allow_cursor_right) {
        cursor_right_btn_ = lv_btn_create(soft_container_);
        lv_obj_t* label = lv_label_create(cursor_right_btn_);
        lv_label_set_text(label, kCursorRightLabel);
        lv_obj_set_size(cursor_right_btn_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    }
    if (params_.allow_previous_page) {
        previous_page_btn_ = lv_btn_create(soft_container_);
        lv_obj_t* label = lv_label_create(previous_page_btn_);
        lv_label_set_text(label, kPreviousPageLabel);
        lv_obj_set_size(previous_page_btn_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    }
    if (params_.show_save_button) {
        save_btn_ = lv_btn_create(soft_container_);
        lv_obj_t* label = lv_label_create(save_btn_);
        lv_label_set_text(label, kSaveLabel);
        lv_obj_set_size(save_btn_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    }
}

// ==================== Layout Helpers ====================

void KeyboardScreen::build_layout_map() {
    layout_strings_.clear();
    layout_map_.clear();
    std::string chars;
    switch (params_.layout) {
        case KeyboardLayout::Lowercase:
            chars = kLowercaseLetters;
            break;
        case KeyboardLayout::Uppercase:
            chars = kUppercaseLetters;
            break;
        case KeyboardLayout::Digits:
            chars = kDigits;
            break;
        case KeyboardLayout::Symbols:
            chars = kSymbols;
            break;
        case KeyboardLayout::Custom:
            if (params_.custom_layout) {
                // Custom layout: rows separated by '|', columns within row as characters
                // For simplicity, treat as flat string for now
                chars = *params_.custom_layout;
                // Remove '|' characters
                chars.erase(std::remove(chars.begin(), chars.end(), '|'), chars.end());
            } else {
                chars = kLowercaseLetters;
            }
            break;
    }
    // Build map: each character becomes a separate button
    for (char c : chars) {
        layout_strings_.push_back(std::string(1, c));
    }
    btn_count_ = static_cast<int>(layout_strings_.size());
    int cols = get_grid_cols();
    int rows = get_grid_rows();
    // Build layout_map_ with row separators (empty strings) for LVGL btnmatrix
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int index = r * cols + c;
            if (index < btn_count_) {
                layout_map_.push_back(layout_strings_[index].c_str());
            } else {
                // Pad with empty strings for incomplete last row
                layout_map_.push_back("");
            }
        }
        // Add row separator (empty string)
        layout_map_.push_back("");
    }
}

int KeyboardScreen::get_grid_rows() const {
    // For simplicity, compute rows based on column count
    int cols = get_grid_cols();
    if (btn_count_ == 0) return 0;
    return (btn_count_ + cols - 1) / cols;
}

int KeyboardScreen::get_grid_cols() const {
    // Use a sensible default; could be configurable later
    return kDefaultCols;
}

// ==================== Input Handling ====================

void KeyboardScreen::move_selection(int dx, int dy) {
    int cols = get_grid_cols();
    int rows = get_grid_rows();
    if (cols == 0 || rows == 0) return;
    int new_row = selected_row_ + dy;
    int new_col = selected_col_ + dx;
    // Wrap around
    if (new_row < 0) new_row = rows - 1;
    if (new_row >= rows) new_row = 0;
    if (new_col < 0) new_col = cols - 1;
    if (new_col >= cols) new_col = 0;
    // Ensure index within button count
    int new_index = new_row * cols + new_col;
    if (new_index >= btn_count_) {
        // Adjust to last button
        new_index = btn_count_ - 1;
        new_row = new_index / cols;
        new_col = new_index % cols;
    }
    // Update checked state
    lv_btnmatrix_clear_btn_ctrl(keyboard_grid_, selected_row_ * cols + selected_col_, LV_BTNMATRIX_CTRL_CHECKED);
    lv_btnmatrix_set_btn_ctrl(keyboard_grid_, new_index, LV_BTNMATRIX_CTRL_CHECKED);
    selected_row_ = new_row;
    selected_col_ = new_col;
}

void KeyboardScreen::press_selected_key() {
    int index = selected_row_ * get_grid_cols() + selected_col_;
    if (index >= btn_count_) return;
    const char* btn_text = layout_map_[index];
    if (!btn_text || strlen(btn_text) == 0) return;
    // Append character to current text
    if (params_.max_length && current_text_.length() >= static_cast<size_t>(*params_.max_length)) {
        // Cannot add more characters
        return;
    }
    current_text_.insert(cursor_position_, btn_text);
    cursor_position_ += static_cast<int>(strlen(btn_text));
    update_text_display();
    emit_text_changed();
}

void KeyboardScreen::press_backspace() {
    if (cursor_position_ > 0) {
        current_text_.erase(cursor_position_ - 1, 1);
        --cursor_position_;
        update_text_display();
        emit_text_changed();
    }
}

void KeyboardScreen::press_space() {
    if (params_.max_length && current_text_.length() >= static_cast<size_t>(*params_.max_length)) {
        return;
    }
    current_text_.insert(cursor_position_, " ");
    ++cursor_position_;
    update_text_display();
    emit_text_changed();
}

void KeyboardScreen::press_cursor_left() {
    if (cursor_position_ > 0) {
        --cursor_position_;
        update_text_display();
        emit_cursor_left();
    }
}

void KeyboardScreen::press_cursor_right() {
    if (cursor_position_ < static_cast<int>(current_text_.size())) {
        ++cursor_position_;
        update_text_display();
        emit_cursor_right();
    }
}

void KeyboardScreen::press_previous_page() {
    // For now, just emit event; could switch layout to previous page (e.g., symbols)
    emit_previous_page();
}

void KeyboardScreen::press_save() {
    emit_save();
}

// ==================== UI Updates ====================

void KeyboardScreen::update_text_display() {
    if (!text_display_) return;
    lv_textarea_set_text(text_display_, current_text_.c_str());
    lv_textarea_set_cursor_pos(text_display_, cursor_position_);
}

void KeyboardScreen::update_keyboard_grid() {
    // Rebuild layout map if layout changed (not needed in this increment)
}

void KeyboardScreen::update_soft_buttons() {
    // Update visibility/enabled state based on params (not needed now)
}

// ==================== Event Emission ====================

void KeyboardScreen::emit_text_changed() {
    context_.emit_action(kTextChangedAction,
                         kKeyboardComponent,
                         EventValue{current_text_});
}

void KeyboardScreen::emit_back() {
    context_.emit_action(kBackAction, kKeyboardComponent);
}

void KeyboardScreen::emit_save() {
    context_.emit_action(kSaveAction,
                         kKeyboardComponent,
                         EventValue{current_text_});
}

void KeyboardScreen::emit_cursor_left() {
    context_.emit_action(kCursorLeftAction, kKeyboardComponent);
}

void KeyboardScreen::emit_cursor_right() {
    context_.emit_action(kCursorRightAction, kKeyboardComponent);
}

void KeyboardScreen::emit_previous_page() {
    context_.emit_action(kPreviousPageAction, kKeyboardComponent);
}

}  // namespace seedsigner::lvgl