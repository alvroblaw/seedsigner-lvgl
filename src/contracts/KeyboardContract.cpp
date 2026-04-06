#include "seedsigner_lvgl/contracts/KeyboardContract.hpp"

#include <charconv>
#include <optional>
#include <string>
#include <string_view>

namespace seedsigner::lvgl {
namespace {

constexpr const char* kLayoutArg = "layout";
constexpr const char* kPlaceholderArg = "placeholder";
constexpr const char* kInitialValueArg = "initial_value";
constexpr const char* kMaxLengthArg = "max_length";
constexpr const char* kShowSaveButtonArg = "show_save_button";
constexpr const char* kAllowBackspaceArg = "allow_backspace";
constexpr const char* kAllowSpaceArg = "allow_space";
constexpr const char* kAllowCursorLeftArg = "allow_cursor_left";
constexpr const char* kAllowCursorRightArg = "allow_cursor_right";
constexpr const char* kAllowPreviousPageArg = "allow_previous_page";
constexpr const char* kCustomLayoutArg = "custom_layout";

constexpr const char* kEventTypeArg = "event";
constexpr const char* kTextArg = "text";
constexpr const char* kCursorPositionArg = "cursor_position";

// Layout string values
constexpr const char* kLayoutLowercase = "lowercase";
constexpr const char* kLayoutUppercase = "uppercase";
constexpr const char* kLayoutDigits = "digits";
constexpr const char* kLayoutSymbols = "symbols";
constexpr const char* kLayoutCustom = "custom";

// Event type strings
constexpr const char* kEventTextChanged = "text_changed";
constexpr const char* kEventBack = "back";
constexpr const char* kEventSave = "save";
constexpr const char* kEventCursorLeft = "cursor_left";
constexpr const char* kEventCursorRight = "cursor_right";
constexpr const char* kEventPreviousPage = "previous_page";

std::optional<int> parse_int(std::string_view str) {
    int value;
    auto result = std::from_chars(str.data(), str.data() + str.size(), value);
    if (result.ec != std::errc{} || result.ptr != str.data() + str.size()) {
        return std::nullopt;
    }
    return value;
}

std::string value_or(const PropertyMap& values, const char* key, const char* fallback = "") {
    const auto it = values.find(key);
    return it == values.end() ? std::string{fallback} : it->second;
}

bool parse_bool(std::string_view str) {
    return str == "1" || str == "true" || str == "yes" || str == "on";
}

}  // namespace

PropertyMap make_keyboard_route_args(const KeyboardParams& params) {
    PropertyMap args;
    args[kLayoutArg] = to_string(params.layout);
    if (params.placeholder) {
        args[kPlaceholderArg] = *params.placeholder;
    }
    if (params.initial_value) {
        args[kInitialValueArg] = *params.initial_value;
    }
    if (params.max_length) {
        args[kMaxLengthArg] = std::to_string(*params.max_length);
    }
    args[kShowSaveButtonArg] = params.show_save_button ? "1" : "0";
    args[kAllowBackspaceArg] = params.allow_backspace ? "1" : "0";
    args[kAllowSpaceArg] = params.allow_space ? "1" : "0";
    args[kAllowCursorLeftArg] = params.allow_cursor_left ? "1" : "0";
    args[kAllowCursorRightArg] = params.allow_cursor_right ? "1" : "0";
    args[kAllowPreviousPageArg] = params.allow_previous_page ? "1" : "0";
    if (params.custom_layout) {
        args[kCustomLayoutArg] = *params.custom_layout;
    }
    return args;
}

KeyboardParams parse_keyboard_params(const PropertyMap& args) {
    KeyboardParams params;
    const std::string layout_str = value_or(args, kLayoutArg, kLayoutLowercase);
    params.layout = parse_keyboard_layout(layout_str);
    
    const auto placeholder_it = args.find(kPlaceholderArg);
    if (placeholder_it != args.end() && !placeholder_it->second.empty()) {
        params.placeholder = placeholder_it->second;
    }
    const auto initial_it = args.find(kInitialValueArg);
    if (initial_it != args.end()) {
        params.initial_value = initial_it->second;
    }
    const auto max_len_it = args.find(kMaxLengthArg);
    if (max_len_it != args.end() && !max_len_it->second.empty()) {
        params.max_length = parse_int(max_len_it->second);
    }
    params.show_save_button = parse_bool(value_or(args, kShowSaveButtonArg, "0"));
    params.allow_backspace = parse_bool(value_or(args, kAllowBackspaceArg, "1"));
    params.allow_space = parse_bool(value_or(args, kAllowSpaceArg, "1"));
    params.allow_cursor_left = parse_bool(value_or(args, kAllowCursorLeftArg, "0"));
    params.allow_cursor_right = parse_bool(value_or(args, kAllowCursorRightArg, "0"));
    params.allow_previous_page = parse_bool(value_or(args, kAllowPreviousPageArg, "0"));
    const auto custom_it = args.find(kCustomLayoutArg);
    if (custom_it != args.end() && !custom_it->second.empty()) {
        params.custom_layout = custom_it->second;
    }
    return params;
}

std::string encode_keyboard_event(const KeyboardEvent& event) {
    std::string encoded;
    switch (event.type) {
        case KeyboardEvent::Type::TextChanged:
            encoded += kEventTypeArg;
            encoded += "=";
            encoded += kEventTextChanged;
            if (event.text) {
                encoded += ";";
                encoded += kTextArg;
                encoded += "=";
                encoded += *event.text;
            }
            if (event.cursor_position) {
                encoded += ";";
                encoded += kCursorPositionArg;
                encoded += "=";
                encoded += std::to_string(*event.cursor_position);
            }
            break;
        case KeyboardEvent::Type::Back:
            encoded += kEventTypeArg;
            encoded += "=";
            encoded += kEventBack;
            break;
        case KeyboardEvent::Type::Save:
            encoded += kEventTypeArg;
            encoded += "=";
            encoded += kEventSave;
            if (event.text) {
                encoded += ";";
                encoded += kTextArg;
                encoded += "=";
                encoded += *event.text;
            }
            break;
        case KeyboardEvent::Type::CursorLeft:
            encoded += kEventTypeArg;
            encoded += "=";
            encoded += kEventCursorLeft;
            break;
        case KeyboardEvent::Type::CursorRight:
            encoded += kEventTypeArg;
            encoded += "=";
            encoded += kEventCursorRight;
            break;
        case KeyboardEvent::Type::PreviousPage:
            encoded += kEventTypeArg;
            encoded += "=";
            encoded += kEventPreviousPage;
            break;
    }
    return encoded;
}

std::string to_string(KeyboardLayout layout) {
    switch (layout) {
        case KeyboardLayout::Lowercase:
            return kLayoutLowercase;
        case KeyboardLayout::Uppercase:
            return kLayoutUppercase;
        case KeyboardLayout::Digits:
            return kLayoutDigits;
        case KeyboardLayout::Symbols:
            return kLayoutSymbols;
        case KeyboardLayout::Custom:
            return kLayoutCustom;
    }
    return kLayoutLowercase;
}

KeyboardLayout parse_keyboard_layout(std::string_view raw) {
    if (raw == kLayoutUppercase) {
        return KeyboardLayout::Uppercase;
    }
    if (raw == kLayoutDigits) {
        return KeyboardLayout::Digits;
    }
    if (raw == kLayoutSymbols) {
        return KeyboardLayout::Symbols;
    }
    if (raw == kLayoutCustom) {
        return KeyboardLayout::Custom;
    }
    // default to lowercase
    return KeyboardLayout::Lowercase;
}

}  // namespace seedsigner::lvgl