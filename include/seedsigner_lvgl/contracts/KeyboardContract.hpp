#pragma once

#include <optional>
#include <string>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

enum class KeyboardLayout {
    Lowercase,
    Uppercase,
    Digits,
    Symbols,
    Custom,
};

struct KeyboardParams {
    KeyboardLayout layout{KeyboardLayout::Lowercase};
    std::optional<std::string> placeholder;
    std::optional<std::string> initial_value;
    std::optional<int> max_length;
    bool show_save_button{false};
    // Soft keys visibility
    bool allow_backspace{true};
    bool allow_space{true};
    bool allow_cursor_left{false};
    bool allow_cursor_right{false};
    bool allow_previous_page{false};
    // Custom layout: provide a string of characters, rows separated by '|'
    std::optional<std::string> custom_layout;
};

struct KeyboardEvent {
    enum class Type {
        TextChanged,
        Back,
        Save,
        CursorLeft,
        CursorRight,
        PreviousPage,
    } type;
    std::optional<std::string> text;
    // For text_changed events, contains the new full text
    std::optional<int> cursor_position;
};

PropertyMap make_keyboard_route_args(const KeyboardParams& params);
KeyboardParams parse_keyboard_params(const PropertyMap& args);
std::string encode_keyboard_event(const KeyboardEvent& event);

// Helper conversions
std::string to_string(KeyboardLayout layout);
KeyboardLayout parse_keyboard_layout(std::string_view raw);

}  // namespace seedsigner::lvgl