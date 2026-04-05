#pragma once

#include <optional>
#include <string>
#include <vector>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

enum class SettingValueType {
    SingleChoice,
    MultiChoice,
};

enum class SettingItemType {
    Choice,
    Toggle,
    Action,
};

struct SettingItemDefinition {
    std::string id;
    std::string label;
    std::string secondary_text;
    SettingItemType item_type{SettingItemType::Choice};
    std::string accessory;
};

struct SettingDefinition {
    std::string id;
    std::string title{"Settings"};
    std::string subtitle;
    std::string section_title;
    std::string help_text;
    std::string footer_text;
    SettingValueType value_type{SettingValueType::SingleChoice};
    std::vector<std::string> default_values;
    std::vector<std::string> current_values;
    std::vector<SettingItemDefinition> items;
};

struct SettingEventPayload {
    std::string event_name;
    std::string setting_id;
    std::string setting_label;
    SettingValueType value_type{SettingValueType::SingleChoice};
    std::vector<std::string> default_values;
    std::vector<std::string> current_values;
    std::size_t index{0};
    SettingItemDefinition item;
    bool is_current{false};
};

std::vector<std::string> parse_setting_value_list(std::string_view raw);
std::string join_setting_value_list(const std::vector<std::string>& values);

std::string to_string(SettingValueType value_type);
std::string to_string(SettingItemType item_type);
SettingValueType parse_setting_value_type(std::string_view raw);
SettingItemType parse_setting_item_type(std::string_view raw);

PropertyMap make_settings_route_args(const SettingDefinition& definition);
PropertyMap make_settings_menu_route_args(const std::vector<SettingDefinition>& definitions);
SettingDefinition parse_setting_definition(const PropertyMap& args);
std::string encode_setting_event_payload(const SettingEventPayload& payload);

}  // namespace seedsigner::lvgl
