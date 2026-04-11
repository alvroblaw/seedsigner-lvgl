#include "seedsigner_lvgl/screens/SettingsMenuScreen.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"

#include "seedsigner_lvgl/components/TopNavBar.hpp"

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace seedsigner::lvgl {

namespace {

constexpr const char* kFocusAction = "focus_changed";
constexpr const char* kSelectAction = "setting_selected";
constexpr const char* kSettingsMenuComponent = "settings_menu";
constexpr const char* kTitleArg = "title";
constexpr const char* kHelpTextArg = "help_text";
constexpr const char* kFooterTextArg = "footer_text";
constexpr const char* kSelectedIndexArg = "selected_index";
constexpr const char* kSettingCountArg = "setting_count";
constexpr const char* kSettingPrefix = "setting_";

constexpr const char* kCheckboxChecked = "[x]";
constexpr const char* kCheckboxUnchecked = "[ ]";

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

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

void SettingsMenuScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    title_ = value_or(route.args, kTitleArg, "Settings");
    help_text_ = value_or(route.args, kHelpTextArg);
    footer_text_ = value_or(route.args, kFooterTextArg);
    definitions_ = parse_definitions(route.args);
    selected_index_ = parse_selected_index(route.args);
    item_buttons_.clear();
    item_primary_labels_.clear();
    item_secondary_labels_.clear();
    item_accessory_labels_.clear();

    if (!styles_initialized_) {
        lv_style_init(&row_style_);
        lv_style_set_radius(&row_style_, seedsigner::lvgl::theme::spacing::ROW_RADIUS);
        lv_style_set_pad_all(&row_style_, seedsigner::lvgl::theme::spacing::ROW_PAD);
        lv_style_set_pad_gap(&row_style_, seedsigner::lvgl::theme::spacing::ROW_GAP);
        lv_style_set_bg_opa(&row_style_, LV_OPA_COVER);
        lv_style_set_bg_color(&row_style_, seedsigner::lvgl::theme::colors::SURFACE_MEDIUM);
        lv_style_set_border_width(&row_style_, 1);
        lv_style_set_border_color(&row_style_, seedsigner::lvgl::theme::colors::BORDER);
        lv_style_set_text_color(&row_style_, seedsigner::lvgl::theme::colors::TEXT_PRIMARY);

        lv_style_init(&selected_row_style_);
        lv_style_set_bg_opa(&selected_row_style_, LV_OPA_50);
        lv_style_set_bg_color(&selected_row_style_, seedsigner::lvgl::theme::colors::PRIMARY);
        lv_style_set_border_width(&selected_row_style_, 2);
        lv_style_set_border_color(&selected_row_style_, seedsigner::lvgl::theme::colors::PRIMARY_LIGHT);
        lv_style_set_outline_width(&selected_row_style_, 2);
        lv_style_set_outline_pad(&selected_row_style_, 1);
        lv_style_set_outline_color(&selected_row_style_, seedsigner::lvgl::theme::colors::PRIMARY);
        styles_initialized_ = true;
    }

    fprintf(stderr, "[SettingsMenuScreen] create: root=%p\n", context.root); fflush(stderr);
    // Root container: column with top nav bar and content area
    container_ = lv_obj_create(context.root);
    fprintf(stderr, "[SettingsMenuScreen] container_=%p\n", container_); fflush(stderr);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_pad_row(container_, 0, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // Top navigation bar
    fprintf(stderr, "[SettingsMenuScreen] creating top nav bar\n"); fflush(stderr);
    top_nav_bar_ = std::make_unique<TopNavBar>(context);
    TopNavBarConfig nav_config;
    nav_config.title = title_;
    nav_config.show_back = true;
    nav_config.show_home = false;
    nav_config.show_cancel = false;
    top_nav_bar_->set_config(nav_config);
    top_nav_bar_->attach(container_);
    fprintf(stderr, "[SettingsMenuScreen] top_nav_bar_=%p\n", top_nav_bar_.get()); fflush(stderr);

    // Content container (scrollable area below the nav bar)
    content_container_ = lv_obj_create(container_);
    fprintf(stderr, "[SettingsMenuScreen] content_container_=%p\n", content_container_); fflush(stderr);
    lv_obj_set_size(content_container_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(content_container_, 1);
    lv_obj_set_scroll_dir(content_container_, LV_DIR_VER);
    lv_obj_set_flex_flow(content_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(content_container_, seedsigner::lvgl::theme::spacing::SCREEN_PADDING, 0);
    lv_obj_set_style_pad_row(content_container_, seedsigner::lvgl::theme::spacing::ROW_GAP, 0);

    if (!help_text_.empty()) {
        help_label_ = lv_label_create(content_container_);
        lv_obj_set_width(help_label_, lv_pct(100));
        lv_label_set_long_mode(help_label_, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_opa(help_label_, LV_OPA_70, 0);
        lv_label_set_text(help_label_, help_text_.c_str());
    }

    list_ = lv_obj_create(content_container_);
    lv_obj_set_size(list_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(list_, 1);
    lv_obj_set_scroll_dir(list_, LV_DIR_VER);
    lv_obj_set_flex_flow(list_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(list_, 0, 0);
    lv_obj_set_style_pad_row(list_, seedsigner::lvgl::theme::spacing::ROW_GAP, 0);

    if (definitions_.empty()) {
        empty_state_ = lv_label_create(list_);
        lv_obj_set_width(empty_state_, lv_pct(100));
        lv_label_set_long_mode(empty_state_, LV_LABEL_LONG_WRAP);
        lv_label_set_text(empty_state_, "No settings provided.");
        return;
    }

    if (selected_index_ >= definitions_.size()) {
        selected_index_ = 0;
    }

    for (std::size_t index = 0; index < definitions_.size(); ++index) {
        const auto& definition = definitions_[index];
        auto* button = lv_btn_create(list_);
        lv_obj_set_width(button, lv_pct(100));
        lv_obj_set_style_min_height(button, definition.help_text.empty() ? 38 : 52, 0);
        lv_obj_set_flex_flow(button, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(button, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_add_style(button, &row_style_, LV_PART_MAIN);
        lv_obj_set_style_bg_color(button, seedsigner::lvgl::theme::colors::SURFACE_LIGHT, LV_STATE_PRESSED);
        lv_obj_set_style_border_color(button, seedsigner::lvgl::theme::colors::PRIMARY_LIGHT, LV_STATE_PRESSED);
        lv_obj_set_style_translate_y(button, 2, LV_STATE_PRESSED);
        lv_obj_set_style_border_width(button, 2, LV_STATE_PRESSED);
        lv_obj_add_event_cb(button, &SettingsMenuScreen::on_item_event, LV_EVENT_FOCUSED, this);
        lv_obj_add_event_cb(button, &SettingsMenuScreen::on_item_event, LV_EVENT_CLICKED, this);

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
        lv_label_set_text(primary, definition.section_title.empty() ? definition.subtitle.empty() ? definition.title.c_str() : definition.subtitle.c_str() : definition.section_title.c_str());

        lv_obj_t* secondary = nullptr;
        if (!definition.help_text.empty()) {
            secondary = lv_label_create(text_column);
            lv_obj_set_width(secondary, lv_pct(100));
            lv_label_set_long_mode(secondary, LV_LABEL_LONG_DOT);
            lv_obj_set_style_text_opa(secondary, LV_OPA_70, 0);
            lv_label_set_text(secondary, definition.help_text.c_str());
        }

        lv_obj_t* accessory = nullptr;
        // Determine accessory based on current values? For now, use chevron.
        const char* glyph = accessory_glyph("chevron");
        if (glyph != nullptr) {
            accessory = lv_label_create(button);
            lv_label_set_text(accessory, glyph);
            lv_obj_set_style_text_align(accessory, LV_TEXT_ALIGN_RIGHT, 0);
        }

        item_buttons_.push_back(button);
        item_primary_labels_.push_back(primary);
        item_secondary_labels_.push_back(secondary);
        item_accessory_labels_.push_back(accessory);
    }

    apply_selection(selected_index_);

    if (!footer_text_.empty()) {
        footer_label_ = lv_label_create(content_container_);
        lv_obj_set_width(footer_label_, lv_pct(100));
        lv_label_set_long_mode(footer_label_, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_opa(footer_label_, LV_OPA_50, 0);
        lv_label_set_text(footer_label_, footer_text_.c_str());
    }
}

void SettingsMenuScreen::destroy() {
    fprintf(stderr, "[SettingsMenuScreen] destroy container_=%p content_container_=%p\n", container_, content_container_); fflush(stderr);
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
    item_primary_labels_.clear();
    item_secondary_labels_.clear();
    item_accessory_labels_.clear();
    definitions_.clear();
    selected_index_ = 0;
    title_.clear();
    help_text_.clear();
    footer_text_.clear();
    context_ = {};
}

bool SettingsMenuScreen::handle_input(const InputEvent& input) {
    fprintf(stderr, "[SettingsMenuScreen] handle_input key=%d top_nav_bar_=%p\n", static_cast<int>(input.key), top_nav_bar_.get()); fflush(stderr);
    // Let TopNavBar handle input first (e.g., hardware back button)
    if (top_nav_bar_ && top_nav_bar_->handle_input(input)) {
        fprintf(stderr, "[SettingsMenuScreen] top_nav_bar consumed input\n"); fflush(stderr);
        return true;
    }

    if (definitions_.empty()) {
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
        emit_setting_selected(context_, selected_index_);
        return true;
    case InputKey::Back:
        // If TopNavBar didn't consume Back (show_back false), emit cancel
        return context_.emit_cancel(kSettingsMenuComponent);
    case InputKey::Left:
    case InputKey::Right:
        return false;
    }

    return false;
}

bool SettingsMenuScreen::set_data(const PropertyMap& data) {
    const auto context = context_;
    destroy();
    create(context, RouteDescriptor{.route_id = context.route_id, .args = data});
    return true;
}

std::vector<SettingDefinition> SettingsMenuScreen::parse_definitions(const PropertyMap& args) {
    std::vector<SettingDefinition> definitions;

    const auto count_it = args.find(kSettingCountArg);
    if (count_it == args.end() || count_it->second.empty()) {
        // Fallback: maybe there's a single definition via legacy args
        // Try to parse as a single setting definition
        SettingDefinition def = parse_setting_definition(args);
        if (!def.id.empty() || !def.items.empty()) {
            definitions.push_back(std::move(def));
        }
        return definitions;
    }

    std::size_t count = 0;
    try {
        count = static_cast<std::size_t>(std::stoul(count_it->second));
    } catch (...) {
        return definitions;
    }

    for (std::size_t i = 0; i < count; ++i) {
        std::string prefix = kSettingPrefix + std::to_string(i) + "_";
        PropertyMap def_args;
        for (const auto& [key, value] : args) {
            if (key.size() > prefix.size() && key.substr(0, prefix.size()) == prefix) {
                std::string subkey = key.substr(prefix.size());
                def_args[subkey] = value;
            }
        }
        // If no subkeys found, skip
        if (def_args.empty()) {
            continue;
        }
        SettingDefinition def = parse_setting_definition(def_args);
        definitions.push_back(std::move(def));
    }

    return definitions;
}

std::size_t SettingsMenuScreen::parse_selected_index(const PropertyMap& args) {
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

std::string SettingsMenuScreen::value_or(const PropertyMap& values, const char* key, const char* fallback) {
    const auto it = values.find(key);
    return it == values.end() ? std::string{fallback} : it->second;
}

void SettingsMenuScreen::apply_selection(std::size_t index) {
    fprintf(stderr, "[SettingsMenuScreen] apply_selection index=%zu item_buttons_.size=%zu\n", index, item_buttons_.size()); fflush(stderr);
    if (item_buttons_.empty()) {
        selected_index_ = 0;
        return;
    }

    selected_index_ = std::min(index, item_buttons_.size() - 1);
    fprintf(stderr, "[SettingsMenuScreen] selected_index_=%zu\n", selected_index_); fflush(stderr);
    for (std::size_t button_index = 0; button_index < item_buttons_.size(); ++button_index) {
        auto* button = item_buttons_[button_index];
        fprintf(stderr, "[SettingsMenuScreen] button %zu = %p\n", button_index, button); fflush(stderr);
        lv_obj_remove_style(button, &selected_row_style_, LV_PART_MAIN);
        if (button_index == selected_index_) {
            lv_obj_add_style(button, &selected_row_style_, LV_PART_MAIN);
            lv_obj_scroll_to_view(button, LV_ANIM_OFF);
        }
    }
}

void SettingsMenuScreen::emit_focus_changed(const ScreenContext& context, std::size_t index) const {
    if (index >= definitions_.size()) {
        return;
    }

    const auto& definition = definitions_[index];
    context.emit_action(kFocusAction,
                        kSettingsMenuComponent,
                        EventValue{static_cast<std::int64_t>(index)},
                        EventMeta{definition.id, std::string{definition.section_title.empty() ? definition.subtitle : definition.section_title}});
}

void SettingsMenuScreen::emit_setting_selected(const ScreenContext& context, std::size_t index) const {
    fprintf(stderr, "[SettingsMenuScreen] emit_setting_selected index=%zu definitions_.size=%zu\n", index, definitions_.size()); fflush(stderr);
    if (index >= definitions_.size()) {
        return;
    }

    const auto& definition = definitions_[index];
    std::string meta_value = definition.section_title.empty() ? definition.subtitle : definition.section_title;
    // Try default_values[0] if exists
    if (!definition.default_values.empty()) {
        meta_value = definition.default_values[0];
    }
    fprintf(stderr, "[SettingsMenuScreen] definition.id=%s meta_value=%s\n", definition.id.c_str(), meta_value.c_str()); fflush(stderr);
    context.emit_action(kSelectAction,
                        kSettingsMenuComponent,
                        EventValue{static_cast<std::int64_t>(index)},
                        EventMeta{definition.id, meta_value});
    fprintf(stderr, "[SettingsMenuScreen] emit_action done\n"); fflush(stderr);
}

const SettingDefinition* SettingsMenuScreen::find_definition(const lv_obj_t* button, std::size_t* index) const noexcept {
    for (std::size_t item_index = 0; item_index < item_buttons_.size(); ++item_index) {
        if (item_buttons_[item_index] == button) {
            if (index != nullptr) {
                *index = item_index;
            }
            return &definitions_[item_index];
        }
    }

    return nullptr;
}

void SettingsMenuScreen::on_item_event(lv_event_t* event) {
    auto* screen = static_cast<SettingsMenuScreen*>(lv_event_get_user_data(event));
    if (screen == nullptr) {
        return;
    }

    std::size_t index = 0;
    if (screen->find_definition(lv_event_get_target(event), &index) == nullptr) {
        return;
    }

    screen->apply_selection(index);

    if (lv_event_get_code(event) == LV_EVENT_FOCUSED) {
        screen->emit_focus_changed(screen->context_, index);
        return;
    }

    if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
        screen->emit_setting_selected(screen->context_, index);
    }
}

}  // namespace seedsigner::lvgl