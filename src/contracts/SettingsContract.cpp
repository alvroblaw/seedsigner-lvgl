#include "seedsigner_lvgl/contracts/SettingsContract.hpp"

#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace seedsigner::lvgl {
namespace {

constexpr const char* kTitleArg = "title";
constexpr const char* kSubtitleArg = "subtitle";
constexpr const char* kSectionTitleArg = "section_title";
constexpr const char* kHelpTextArg = "help_text";
constexpr const char* kFooterTextArg = "footer_text";
constexpr const char* kItemsArg = "items";
constexpr const char* kSelectionModeArg = "selection_mode";
constexpr const char* kCurrentValueArg = "current_value";
constexpr const char* kCurrentValuesArg = "current_values";
constexpr const char* kDefaultValueArg = "default_value";
constexpr const char* kDefaultValuesArg = "default_values";
constexpr const char* kSettingIdArg = "setting_id";
constexpr const char* kSettingLabelArg = "setting_label";
constexpr const char* kSettingTypeArg = "setting_type";

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string value_or(const PropertyMap& values, const char* key, const char* fallback = "") {
    const auto it = values.find(key);
    return it == values.end() ? std::string{fallback} : it->second;
}

}  // namespace

std::vector<std::string> parse_setting_value_list(std::string_view raw) {
    std::vector<std::string> values;
    std::string current;
    for (char ch : raw) {
        if (ch == ',' || ch == '\n' || ch == ';') {
            auto trimmed = trim(current);
            if (!trimmed.empty()) {
                values.push_back(std::move(trimmed));
            }
            current.clear();
            continue;
        }
        current.push_back(ch);
    }

    auto trimmed = trim(current);
    if (!trimmed.empty()) {
        values.push_back(std::move(trimmed));
    }
    return values;
}

std::string join_setting_value_list(const std::vector<std::string>& values) {
    std::string joined;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            joined += ',';
        }
        joined += values[i];
    }
    return joined;
}

std::string to_string(SettingValueType value_type) {
    return value_type == SettingValueType::MultiChoice ? "multi" : "single";
}

std::string to_string(SettingItemType item_type) {
    switch (item_type) {
    case SettingItemType::Choice:
        return "choice";
    case SettingItemType::Toggle:
        return "toggle";
    case SettingItemType::Action:
        return "action";
    }
    return "choice";
}

SettingValueType parse_setting_value_type(std::string_view raw) {
    return raw == "multi" ? SettingValueType::MultiChoice : SettingValueType::SingleChoice;
}

SettingItemType parse_setting_item_type(std::string_view raw) {
    if (raw == "toggle") {
        return SettingItemType::Toggle;
    }
    if (raw == "action") {
        return SettingItemType::Action;
    }
    return SettingItemType::Choice;
}

PropertyMap make_settings_route_args(const SettingDefinition& definition) {
    PropertyMap args;
    args[kTitleArg] = definition.title;
    if (!definition.subtitle.empty()) {
        args[kSubtitleArg] = definition.subtitle;
    }
    if (!definition.section_title.empty()) {
        args[kSectionTitleArg] = definition.section_title;
    }
    if (!definition.help_text.empty()) {
        args[kHelpTextArg] = definition.help_text;
    }
    if (!definition.footer_text.empty()) {
        args[kFooterTextArg] = definition.footer_text;
    }

    args[kSelectionModeArg] = to_string(definition.value_type);
    args[kSettingTypeArg] = to_string(definition.value_type);
    if (!definition.id.empty()) {
        args[kSettingIdArg] = definition.id;
    }
    args[kSettingLabelArg] = !definition.section_title.empty() ? definition.section_title : definition.subtitle;

    if (definition.value_type == SettingValueType::MultiChoice) {
        if (!definition.default_values.empty()) {
            args[kDefaultValuesArg] = join_setting_value_list(definition.default_values);
        }
        if (!definition.current_values.empty()) {
            args[kCurrentValuesArg] = join_setting_value_list(definition.current_values);
        }
    } else {
        if (!definition.default_values.empty()) {
            args[kDefaultValueArg] = definition.default_values.front();
        }
        if (!definition.current_values.empty()) {
            args[kCurrentValueArg] = definition.current_values.front();
        }
    }

    std::string encoded_items;
    for (std::size_t i = 0; i < definition.items.size(); ++i) {
        const auto& item = definition.items[i];
        if (i != 0) {
            encoded_items += '\n';
        }
        encoded_items += item.id;
        encoded_items += '|';
        encoded_items += item.label.empty() ? item.id : item.label;
        encoded_items += '|';
        encoded_items += item.secondary_text;
        encoded_items += '|';
        encoded_items += to_string(item.item_type);
        encoded_items += '|';
        encoded_items += item.accessory;
    }
    args[kItemsArg] = std::move(encoded_items);
    return args;
}

SettingDefinition parse_setting_definition(const PropertyMap& args) {
    SettingDefinition definition;
    definition.id = value_or(args, kSettingIdArg);
    definition.title = value_or(args, kTitleArg, "Settings");
    definition.subtitle = value_or(args, kSubtitleArg);
    definition.section_title = value_or(args, kSectionTitleArg);
    definition.help_text = value_or(args, kHelpTextArg);
    definition.footer_text = value_or(args, kFooterTextArg);
    const auto setting_type = value_or(args, kSettingTypeArg);
    definition.value_type = parse_setting_value_type(setting_type.empty() ? value_or(args, kSelectionModeArg, "single") : setting_type);

    if (definition.value_type == SettingValueType::MultiChoice) {
        definition.default_values = parse_setting_value_list(value_or(args, kDefaultValuesArg));
        definition.current_values = parse_setting_value_list(value_or(args, kCurrentValuesArg));
    } else {
        const auto default_value = value_or(args, kDefaultValueArg);
        if (!default_value.empty()) {
            definition.default_values.push_back(default_value);
        }
        const auto current_value = value_or(args, kCurrentValueArg);
        if (!current_value.empty()) {
            definition.current_values.push_back(current_value);
        }
    }

    const auto items_it = args.find(kItemsArg);
    if (items_it == args.end() || items_it->second.empty()) {
        return definition;
    }

    std::stringstream lines{items_it->second};
    std::string line;
    while (std::getline(lines, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        std::vector<std::string> parts;
        std::stringstream columns{line};
        std::string part;
        while (std::getline(columns, part, '|')) {
            parts.push_back(trim(part));
        }
        if (parts.empty() || parts[0].empty()) {
            continue;
        }

        SettingItemDefinition item{.id = parts[0],
                                   .label = parts.size() >= 2 && !parts[1].empty() ? parts[1] : parts[0],
                                   .secondary_text = parts.size() >= 3 ? parts[2] : std::string{},
                                   .item_type = parse_setting_item_type(parts.size() >= 4 ? parts[3] : std::string_view{}),
                                   .accessory = parts.size() >= 5 ? parts[4] : (parts.size() == 4 ? parts[3] : std::string{})};
        definition.items.push_back(std::move(item));
    }

    return definition;
}

std::string encode_setting_event_payload(const SettingEventPayload& payload) {
    std::string encoded;
    encoded += "event=";
    encoded += payload.event_name;
    encoded += ";setting_id=";
    encoded += payload.setting_id;
    encoded += ";setting_label=";
    encoded += payload.setting_label;
    encoded += ";setting_type=";
    encoded += to_string(payload.value_type);
    encoded += ";index=";
    encoded += std::to_string(payload.index);
    encoded += ";item_id=";
    encoded += payload.item.id;
    encoded += ";item_label=";
    encoded += payload.item.label;
    encoded += ";item_type=";
    encoded += to_string(payload.item.item_type);
    encoded += ";is_current=";
    encoded += payload.is_current ? "true" : "false";
    encoded += ";default_value=";
    encoded += payload.value_type == SettingValueType::SingleChoice && !payload.default_values.empty() ? payload.default_values.front() : std::string{};
    encoded += ";default_values=";
    encoded += join_setting_value_list(payload.default_values);
    encoded += ";current_value=";
    encoded += payload.value_type == SettingValueType::SingleChoice && !payload.current_values.empty() ? payload.current_values.front() : std::string{};
    encoded += ";current_values=";
    encoded += join_setting_value_list(payload.current_values);
    return encoded;
}

}  // namespace seedsigner::lvgl
