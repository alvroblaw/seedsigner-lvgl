#include "seedsigner_lvgl/screens/SettingsSelectionScreen.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"

#include "seedsigner_lvgl/components/TopNavBar.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>

namespace seedsigner::lvgl {

namespace {

constexpr const char* kFocusAction = "focus_changed";
constexpr const char* kSelectAction = "setting_selected";
constexpr const char* kSettingsComponent = "settings_selection";
constexpr const char* kTitleArg = "title";
constexpr const char* kSubtitleArg = "subtitle";
constexpr const char* kSectionTitleArg = "section_title";
constexpr const char* kHelpTextArg = "help_text";
constexpr const char* kFooterTextArg = "footer_text";
constexpr const char* kSelectedIndexArg = "selected_index";
constexpr const char* kCurrentValueArg = "current_value";
constexpr const char* kCurrentValuesArg = "current_values";
constexpr const char* kDefaultValueArg = "default_value";
constexpr const char* kDefaultValuesArg = "default_values";
constexpr const char* kCheckboxChecked = "[x]";
constexpr const char* kCheckboxUnchecked = "[ ]";

const char* accessory_glyph(std::string_view accessory) {
    if (accessory == "check" || accessory == "checked" || accessory == "selected") {
        return LV_SYMBOL_OK;
    }
    if (accessory == "chevron" || accessory == "next") {
        return LV_SYMBOL_RIGHT;
    }
    if (accessory == "toggle_on") {
        return LV_SYMBOL_OK " on";
    }
    if (accessory == "checkbox" || accessory == "unchecked") {
        return kCheckboxUnchecked;
    }
    if (accessory == "checkbox_checked") {
        return kCheckboxChecked;
    }
    return nullptr;
}

}  // namespace

void SettingsSelectionScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    definition_ = parse_setting_definition(route.args);
    title_ = definition_.title;
    subtitle_ = definition_.subtitle;
    section_title_ = definition_.section_title;
    help_text_ = definition_.help_text;
    footer_text_ = definition_.footer_text;
    items_ = definition_.items;
    selected_index_ = parse_selected_index(route.args);
    apply_current_values_from_route(route.args);
    item_buttons_.clear();
    item_accessory_labels_.clear();

    if (!styles_initialized_) {
        lv_style_init(&row_style_);
        lv_style_set_radius(&row_style_, 8);
        lv_style_set_pad_all(&row_style_, 10);
        lv_style_set_pad_gap(&row_style_, 8);
        lv_style_set_bg_opa(&row_style_, LV_OPA_TRANSP);
        lv_style_set_border_width(&row_style_, 1);
        lv_style_set_border_color(&row_style_, seedsigner::lvgl::theme::colors::BORDER);

        lv_style_init(&selected_row_style_);
        lv_style_set_bg_opa(&selected_row_style_, LV_OPA_20);
        lv_style_set_bg_color(&selected_row_style_, seedsigner::lvgl::theme::colors::PRIMARY);
        lv_style_set_border_width(&selected_row_style_, 2);
        lv_style_set_border_color(&selected_row_style_, seedsigner::lvgl::theme::colors::PRIMARY);
        styles_initialized_ = true;
    }

    // Root container: column with top nav bar and content area
    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_pad_row(container_, 0, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // Top navigation bar
    top_nav_bar_ = std::make_unique<TopNavBar>(context);
    TopNavBarConfig nav_config;
    nav_config.title = title_;
    nav_config.show_back = true;
    nav_config.show_home = false;
    nav_config.show_cancel = false;
    top_nav_bar_->set_config(nav_config);
    top_nav_bar_->attach(container_);

    // Content container (scrollable area below the nav bar)
    content_container_ = lv_obj_create(container_);
    lv_obj_set_size(content_container_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(content_container_, 1);
    lv_obj_set_scroll_dir(content_container_, LV_DIR_VER);
    lv_obj_set_flex_flow(content_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(content_container_, 12, 0);
    lv_obj_set_style_pad_row(content_container_, 8, 0);

    // Subtitle and section title (if any) go inside content container
    if (!subtitle_.empty()) {
        auto* subtitle = lv_label_create(content_container_);
        lv_obj_set_width(subtitle, lv_pct(100));
        lv_label_set_long_mode(subtitle, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_opa(subtitle, LV_OPA_70, 0);
        lv_label_set_text(subtitle, subtitle_.c_str());
    }

    if (!section_title_.empty()) {
        auto* section = lv_label_create(content_container_);
        lv_obj_set_width(section, lv_pct(100));
        lv_obj_set_style_text_opa(section, LV_OPA_70, 0);
        lv_label_set_text(section, section_title_.c_str());
    }

    list_ = lv_obj_create(content_container_);
    lv_obj_set_size(list_, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(list_, 1);
    lv_obj_set_scroll_dir(list_, LV_DIR_VER);
    lv_obj_set_flex_flow(list_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(list_, 0, 0);
    lv_obj_set_style_pad_row(list_, 8, 0);

    if (items_.empty()) {
        empty_state_ = lv_label_create(list_);
        lv_obj_set_width(empty_state_, lv_pct(100));
        lv_label_set_long_mode(empty_state_, LV_LABEL_LONG_WRAP);
        lv_label_set_text(empty_state_, "No settings options provided.");
    } else {
        if (selected_index_ >= items_.size()) {
            selected_index_ = 0;
        }

        for (std::size_t index = 0; index < items_.size(); ++index) {
            const auto& item = items_[index];
            auto* button = lv_btn_create(list_);
            lv_obj_set_width(button, lv_pct(100));
            lv_obj_set_style_min_height(button, item.secondary_text.empty() ? 46 : 64, 0);
            lv_obj_set_flex_flow(button, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(button, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_add_style(button, &row_style_, LV_PART_MAIN);
            lv_obj_add_event_cb(button, &SettingsSelectionScreen::on_item_event, LV_EVENT_FOCUSED, this);
            lv_obj_add_event_cb(button, &SettingsSelectionScreen::on_item_event, LV_EVENT_CLICKED, this);

            auto* text_column = lv_obj_create(button);
            lv_obj_set_flex_grow(text_column, 1);
            lv_obj_set_height(text_column, LV_SIZE_CONTENT);
            lv_obj_set_style_bg_opa(text_column, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(text_column, 0, 0);
            lv_obj_set_style_pad_all(text_column, 0, 0);
            lv_obj_set_style_pad_row(text_column, 2, 0);
            lv_obj_set_flex_flow(text_column, LV_FLEX_FLOW_COLUMN);
            lv_obj_clear_flag(text_column, LV_OBJ_FLAG_SCROLLABLE);

            auto* primary = lv_label_create(text_column);
            lv_obj_set_width(primary, lv_pct(100));
            lv_label_set_long_mode(primary, LV_LABEL_LONG_DOT);
            lv_label_set_text(primary, item.label.c_str());

            if (!item.secondary_text.empty()) {
                auto* secondary = lv_label_create(text_column);
                lv_obj_set_width(secondary, lv_pct(100));
                lv_label_set_long_mode(secondary, LV_LABEL_LONG_DOT);
                lv_obj_set_style_text_opa(secondary, LV_OPA_70, 0);
                lv_label_set_text(secondary, item.secondary_text.c_str());
            }

            auto* accessory = lv_label_create(button);
            lv_label_set_text(accessory, accessory_text_for_item(item));
            lv_obj_set_style_text_align(accessory, LV_TEXT_ALIGN_RIGHT, 0);

            item_buttons_.push_back(button);
            item_accessory_labels_.push_back(accessory);
        }

        apply_selection(selected_index_);
        refresh_item_accessories();
    }

    if (!help_text_.empty()) {
        help_label_ = lv_label_create(container_);
        lv_obj_set_width(help_label_, lv_pct(100));
        lv_label_set_long_mode(help_label_, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_opa(help_label_, LV_OPA_70, 0);
        lv_label_set_text(help_label_, help_text_.c_str());
    }

    if (!footer_text_.empty()) {
        footer_label_ = lv_label_create(container_);
        lv_obj_set_width(footer_label_, lv_pct(100));
        lv_label_set_long_mode(footer_label_, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_opa(footer_label_, LV_OPA_50, 0);
        lv_label_set_text(footer_label_, footer_text_.c_str());
    }
}

void SettingsSelectionScreen::destroy() {
    top_nav_bar_.reset();
    if (container_ != nullptr) {
        lv_obj_del(container_);
    }

    container_ = nullptr;
    content_container_ = nullptr;
    list_ = nullptr;
    empty_state_ = nullptr;
    help_label_ = nullptr;
    footer_label_ = nullptr;
    item_buttons_.clear();
    item_accessory_labels_.clear();
    items_.clear();
    selected_index_ = 0;
    title_.clear();
    subtitle_.clear();
    section_title_.clear();
    help_text_.clear();
    footer_text_.clear();
    definition_ = {};
    current_value_id_.clear();
    current_value_ids_set_.clear();
    context_ = {};
}

bool SettingsSelectionScreen::handle_input(const InputEvent& input) {
    // Let TopNavBar handle input first (e.g., hardware back button)
    if (top_nav_bar_ && top_nav_bar_->handle_input(input)) {
        return true;
    }

    if (items_.empty()) {
        return false;
    }

    switch (input.key) {
    case InputKey::Up:
        apply_selection(selected_index_ == 0 ? item_buttons_.size() - 1 : selected_index_ - 1);
        emit_focus_changed(context_, selected_index_);
        return true;
    case InputKey::Down:
        apply_selection((selected_index_ + 1) % item_buttons_.size());
        emit_focus_changed(context_, selected_index_);
        return true;
    case InputKey::Press:
        toggle_current_value(selected_index_);
        refresh_item_accessories();
        emit_item_selected(context_, selected_index_);
        return true;
    case InputKey::Back:
        // If TopNavBar didn't consume Back (show_back false), emit cancel
        return context_.emit_cancel(kSettingsComponent);
    case InputKey::Left:
    case InputKey::Right:
        return false;
    }

    return false;
}

bool SettingsSelectionScreen::set_data(const PropertyMap& data) {
    const auto context = context_;
    destroy();
    create(context, RouteDescriptor{.route_id = context.route_id, .args = data});
    return true;
}

std::size_t SettingsSelectionScreen::parse_selected_index(const PropertyMap& args) {
    const auto it = args.find(kSelectedIndexArg);
    if (it == args.end() || it->second.empty()) {
        return 0;
    }

    try {
        return static_cast<std::size_t>(std::stoul(it->second));
    } catch (...) {
        return 0;
    }
}

std::string SettingsSelectionScreen::value_or(const PropertyMap& values, const char* key, const char* fallback) {
    const auto it = values.find(key);
    return it == values.end() ? std::string{fallback} : it->second;
}

void SettingsSelectionScreen::apply_current_values_from_route(const PropertyMap& args) {
    current_value_id_.clear();
    current_value_ids_set_.clear();

    if (definition_.value_type == SettingValueType::MultiChoice) {
        auto current_values = definition_.current_values;
        if (current_values.empty()) {
            const auto fallback_values = value_or(args, kDefaultValuesArg);
            current_values = parse_setting_value_list(current_values.empty() ? value_or(args, kCurrentValuesArg, fallback_values.c_str()) : std::string{});
        }
        for (const auto& id : current_values) {
            current_value_ids_set_.insert(id);
        }
        return;
    }

    if (!definition_.current_values.empty()) {
        current_value_id_ = definition_.current_values.front();
    } else {
        const auto fallback_value = value_or(args, kDefaultValueArg);
        current_value_id_ = value_or(args, kCurrentValueArg, fallback_value.c_str());
    }
    if (current_value_id_.empty() && selected_index_ < items_.size()) {
        current_value_id_ = items_[selected_index_].id;
    }
}

void SettingsSelectionScreen::apply_selection(std::size_t index) {
    if (item_buttons_.empty()) {
        selected_index_ = 0;
        return;
    }

    selected_index_ = std::min(index, item_buttons_.size() - 1);
    for (std::size_t button_index = 0; button_index < item_buttons_.size(); ++button_index) {
        auto* button = item_buttons_[button_index];
        lv_obj_remove_style(button, &selected_row_style_, LV_PART_MAIN);
        if (button_index == selected_index_) {
            lv_obj_add_style(button, &selected_row_style_, LV_PART_MAIN);
            lv_obj_scroll_to_view(button, LV_ANIM_OFF);
        }
    }
}

void SettingsSelectionScreen::emit_focus_changed(const ScreenContext& context, std::size_t index) const {
    if (index >= items_.size()) {
        return;
    }

    context.emit_action(kFocusAction,
                        kSettingsComponent,
                        EventValue{static_cast<std::int64_t>(index)},
                        EventMeta{items_[index].id, payload_for_item(index, "focus")});
}

void SettingsSelectionScreen::emit_item_selected(const ScreenContext& context, std::size_t index) const {
    if (index >= items_.size()) {
        return;
    }

    context.emit_action(kSelectAction,
                        kSettingsComponent,
                        EventValue{static_cast<std::int64_t>(index)},
                        EventMeta{items_[index].id, payload_for_item(index, "select")});
}

void SettingsSelectionScreen::toggle_current_value(std::size_t index) {
    if (index >= items_.size()) {
        return;
    }

    const auto& id = items_[index].id;
    if (definition_.value_type == SettingValueType::MultiChoice) {
        if (current_value_ids_set_.count(id) != 0U) {
            current_value_ids_set_.erase(id);
        } else {
            current_value_ids_set_.insert(id);
        }
        return;
    }

    current_value_id_ = id;
}

bool SettingsSelectionScreen::is_current_value(std::string_view id) const noexcept {
    if (definition_.value_type == SettingValueType::MultiChoice) {
        return current_value_ids_set_.count(std::string{id}) != 0U;
    }
    return current_value_id_ == id;
}

std::vector<std::string> SettingsSelectionScreen::current_value_ids() const {
    if (definition_.value_type == SettingValueType::MultiChoice) {
        std::vector<std::string> ids;
        ids.reserve(current_value_ids_set_.size());
        for (const auto& item : items_) {
            if (current_value_ids_set_.count(item.id) != 0U) {
                ids.push_back(item.id);
            }
        }
        return ids;
    }

    if (current_value_id_.empty()) {
        return {};
    }
    return {current_value_id_};
}

std::string SettingsSelectionScreen::payload_for_item(std::size_t index, std::string_view event_name) const {
    return encode_setting_event_payload(SettingEventPayload{.event_name = std::string{event_name},
                                                            .setting_id = definition_.id,
                                                            .setting_label = !definition_.section_title.empty() ? definition_.section_title : definition_.subtitle,
                                                            .value_type = definition_.value_type,
                                                            .default_values = definition_.default_values,
                                                            .current_values = current_value_ids(),
                                                            .index = index,
                                                            .item = items_[index],
                                                            .is_current = is_current_value(items_[index].id)});
}

void SettingsSelectionScreen::refresh_item_accessories() {
    for (std::size_t index = 0; index < items_.size() && index < item_accessory_labels_.size(); ++index) {
        if (item_accessory_labels_[index] == nullptr) {
            continue;
        }
        lv_label_set_text(item_accessory_labels_[index], accessory_text_for_item(items_[index]));
    }
}

const char* SettingsSelectionScreen::accessory_text_for_item(const Item& item) const {
    if (definition_.value_type == SettingValueType::MultiChoice || item.item_type == SettingItemType::Toggle) {
        return is_current_value(item.id) ? kCheckboxChecked : kCheckboxUnchecked;
    }

    if (is_current_value(item.id)) {
        return LV_SYMBOL_OK;
    }

    if (item.accessory.empty()) {
        return item.item_type == SettingItemType::Action ? LV_SYMBOL_RIGHT : "";
    }

    const char* glyph = accessory_glyph(item.accessory);
    return glyph != nullptr ? glyph : item.accessory.c_str();
}

const SettingsSelectionScreen::Item* SettingsSelectionScreen::find_item(const lv_obj_t* button, std::size_t* index) const noexcept {
    for (std::size_t item_index = 0; item_index < item_buttons_.size(); ++item_index) {
        if (item_buttons_[item_index] == button) {
            if (index != nullptr) {
                *index = item_index;
            }
            return &items_[item_index];
        }
    }

    return nullptr;
}

void SettingsSelectionScreen::on_item_event(lv_event_t* event) {
    auto* screen = static_cast<SettingsSelectionScreen*>(lv_event_get_user_data(event));
    if (screen == nullptr) {
        return;
    }

    std::size_t index = 0;
    if (screen->find_item(lv_event_get_target(event), &index) == nullptr) {
        return;
    }

    screen->apply_selection(index);

    if (lv_event_get_code(event) == LV_EVENT_FOCUSED) {
        screen->emit_focus_changed(screen->context_, index);
        return;
    }

    if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
        screen->toggle_current_value(index);
        screen->refresh_item_accessories();
        screen->emit_item_selected(screen->context_, index);
    }
}

}  // namespace seedsigner::lvgl
