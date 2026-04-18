#include "seedsigner_lvgl/screens/MenuListScreen.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"
#include "icons.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace seedsigner::lvgl {

namespace {

constexpr const char* kFocusAction = "focus_changed";
constexpr const char* kSelectAction = "item_selected";
constexpr const char* kMenuComponent = "menu_list";
constexpr const char* kTitleArg = "title";
constexpr const char* kItemsArg = "items";
constexpr const char* kSelectedIndexArg = "selected_index";

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

const lv_img_dsc_t* accessory_icon(std::string_view accessory) {
    if (accessory == "check" || accessory == "checked" || accessory == "selected") {
        return &img_check;
    }
    if (accessory == "chevron" || accessory == "next") {
        return &img_arrow_right;
    }
    return nullptr;
}

const char* accessory_glyph(std::string_view accessory) {
    if (accessory == "toggle_on") {
        return LV_SYMBOL_OK " on";
    }
    return nullptr;
}

}  // namespace

void MenuListScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    title_ = value_or(route.args, kTitleArg, "Menu");
    items_ = parse_items(route.args);
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
        lv_style_set_bg_color(&row_style_, seedsigner::lvgl::theme::active_theme().SURFACE_MEDIUM);
        lv_style_set_border_width(&row_style_, 1);
        lv_style_set_border_color(&row_style_, seedsigner::lvgl::theme::active_theme().BORDER);
        lv_style_set_text_color(&row_style_, seedsigner::lvgl::theme::active_theme().TEXT_PRIMARY);

        lv_style_init(&selected_row_style_);
        lv_style_set_bg_opa(&selected_row_style_, LV_OPA_50);
        lv_style_set_bg_color(&selected_row_style_, seedsigner::lvgl::theme::active_theme().PRIMARY);
        lv_style_set_border_width(&selected_row_style_, 2);
        lv_style_set_border_color(&selected_row_style_, seedsigner::lvgl::theme::active_theme().PRIMARY_LIGHT);
        lv_style_set_outline_width(&selected_row_style_, 2);
        lv_style_set_outline_pad(&selected_row_style_, 1);
        lv_style_set_outline_color(&selected_row_style_, seedsigner::lvgl::theme::active_theme().PRIMARY);
        styles_initialized_ = true;
    }

    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    // No padding on root container; TopNavBar and content container manage their own spacing.

    // Top navigation bar
    top_nav_bar_ = std::make_unique<TopNavBar>(context_);
    TopNavBarConfig nav_config;
    nav_config.title = title_;
    nav_config.show_back = true;
    nav_config.show_home = false;
    nav_config.show_cancel = false;
    // No custom actions
    top_nav_bar_->set_config(nav_config);
    top_nav_bar_->attach(container_);

    // Content container (everything below the nav bar)
    content_container_ = lv_obj_create(container_);
    lv_obj_set_size(content_container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(content_container_, seedsigner::lvgl::theme::spacing::SCREEN_PADDING, 0);
    lv_obj_set_style_pad_row(content_container_, seedsigner::lvgl::theme::spacing::ROW_GAP, 0);
    lv_obj_set_flex_flow(content_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_flex_grow(content_container_, 1); // Take remaining height

    list_ = lv_obj_create(content_container_);
    lv_obj_set_size(list_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(list_, 1);
    lv_obj_set_scroll_dir(list_, LV_DIR_VER);
    lv_obj_set_flex_flow(list_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(list_, 0, 0);
    lv_obj_set_style_pad_row(list_, seedsigner::lvgl::theme::spacing::ROW_GAP, 0);

    if (items_.empty()) {
        empty_state_ = lv_label_create(list_);
        lv_obj_set_width(empty_state_, lv_pct(100));
        lv_label_set_long_mode(empty_state_, LV_LABEL_LONG_WRAP);
        lv_label_set_text(empty_state_, "No menu items provided.");
        return;
    }

    // Determine section header label (use label field if non-empty, else uppercase id)
    // Items with is_section_header=true get rendered as dividers.
    std::size_t selectable_count = 0;

    if (selected_index_ >= items_.size()) {
        selected_index_ = 0;
    }

    for (std::size_t index = 0; index < items_.size(); ++index) {
        const auto& item = items_[index];

        // --- Section header row ---
        if (item.is_section_header) {
            auto* header = lv_obj_create(list_);
            lv_obj_set_size(header, lv_pct(100), LV_SIZE_CONTENT);
            lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(header, 0, 0);
            lv_obj_set_style_pad_all(header, 2, 0);
            lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

            auto* lbl = lv_label_create(header);
            std::string header_text = item.label.empty() ? item.id : item.label;
            // Convert to uppercase for section headers
            std::transform(header_text.begin(), header_text.end(), header_text.begin(), ::toupper);
            lv_label_set_text(lbl, header_text.c_str());
            lv_obj_set_style_text_font(lbl, seedsigner::lvgl::theme::typography::CAPTION, 0);
            lv_obj_set_style_text_color(lbl, seedsigner::lvgl::theme::active_theme().TEXT_SECONDARY, 0);

            item_buttons_.push_back(nullptr);  // Placeholder to keep index alignment
            item_primary_labels_.push_back(lbl);
            item_secondary_labels_.push_back(nullptr);
            item_accessory_labels_.push_back(nullptr);
            continue;
        }

        ++selectable_count;
        auto* button = lv_btn_create(list_);
        lv_obj_set_width(button, lv_pct(100));
        lv_obj_set_style_min_height(button, item.secondary_text.empty() ? theme::spacing::MENU_ROW_HEIGHT : theme::spacing::MENU_ROW_HEIGHT_TWO_LINE, 0);
        lv_obj_set_flex_flow(button, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(button, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_add_style(button, &row_style_, LV_PART_MAIN);
        lv_obj_set_style_bg_color(button, seedsigner::lvgl::theme::active_theme().SURFACE_LIGHT, LV_STATE_PRESSED);
        lv_obj_set_style_border_color(button, seedsigner::lvgl::theme::active_theme().PRIMARY_LIGHT, LV_STATE_PRESSED);
        lv_obj_set_style_translate_y(button, 2, LV_STATE_PRESSED);
        lv_obj_set_style_border_width(button, 2, LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(button, LV_OPA_50, LV_STATE_FOCUSED);
        lv_obj_set_style_bg_color(button, seedsigner::lvgl::theme::active_theme().PRIMARY, LV_STATE_FOCUSED);
        lv_obj_set_style_border_width(button, 2, LV_STATE_FOCUSED);
        lv_obj_set_style_border_color(button, seedsigner::lvgl::theme::active_theme().PRIMARY_LIGHT, LV_STATE_FOCUSED);
        lv_obj_set_style_outline_width(button, 2, LV_STATE_FOCUSED);
        lv_obj_set_style_outline_pad(button, 1, LV_STATE_FOCUSED);
        lv_obj_set_style_outline_color(button, seedsigner::lvgl::theme::active_theme().PRIMARY, LV_STATE_FOCUSED);
        lv_obj_add_event_cb(button, &MenuListScreen::on_item_event, LV_EVENT_FOCUSED, this);
        lv_obj_add_event_cb(button, &MenuListScreen::on_item_event, LV_EVENT_CLICKED, this);

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

        lv_obj_t* secondary = nullptr;
        if (!item.secondary_text.empty()) {
            secondary = lv_label_create(text_column);
            lv_obj_set_width(secondary, lv_pct(100));
            lv_label_set_long_mode(secondary, LV_LABEL_LONG_DOT);
            lv_obj_set_style_text_opa(secondary, LV_OPA_70, 0);
            lv_label_set_text(secondary, item.secondary_text.c_str());
        }

        lv_obj_t* accessory = nullptr;
        if (!item.accessory.empty()) {
            const lv_img_dsc_t* icon = accessory_icon(item.accessory);
            if (icon != nullptr) {
                accessory = lv_img_create(button);
                lv_img_set_src(accessory, icon);
                lv_obj_set_style_img_recolor(accessory, seedsigner::lvgl::theme::active_theme().TEXT_PRIMARY, 0);
                lv_obj_set_style_img_recolor_opa(accessory, LV_OPA_COVER, 0);
            } else {
                const char* glyph = accessory_glyph(item.accessory);
                accessory = lv_label_create(button);
                lv_label_set_text(accessory, glyph != nullptr ? glyph : item.accessory.c_str());
            }
            lv_obj_set_style_text_align(accessory, LV_TEXT_ALIGN_RIGHT, 0);
        }

        item_buttons_.push_back(button);
        item_primary_labels_.push_back(primary);
        item_secondary_labels_.push_back(secondary);
        item_accessory_labels_.push_back(accessory);
    }

    apply_selection(selected_index_);

    // --- Scroll position indicator ---
    // A thin vertical bar on the right edge showing scroll progress when list overflows
    if (selectable_count > 0) {
        scrollbar_ = lv_obj_create(content_container_);
        lv_obj_set_size(scrollbar_, 3, lv_pct(100));
        lv_obj_align(scrollbar_, LV_ALIGN_TOP_RIGHT, -2, 0);
        lv_obj_set_style_bg_opa(scrollbar_, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(scrollbar_, 0, 0);
        lv_obj_clear_flag(scrollbar_, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_pad_all(scrollbar_, 0, 0);

        // Inner thumb
        auto* thumb = lv_obj_create(scrollbar_);
        lv_obj_set_size(thumb, 3, 20);  // Default height, updated on scroll
        lv_obj_align(thumb, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_color(thumb, seedsigner::lvgl::theme::active_theme().BORDER, 0);
        lv_obj_set_style_bg_opa(thumb, LV_OPA_60, 0);
        lv_obj_set_style_radius(thumb, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(thumb, 0, 0);
        lv_obj_clear_flag(thumb, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    }
}

void MenuListScreen::destroy() {
    // TopNavBar must be cleared before its parent container is deleted.
    top_nav_bar_.reset();
    if (container_ != nullptr) {
        lv_obj_del(container_);
    }

    container_ = nullptr;
    content_container_ = nullptr;
    list_ = nullptr;
    scrollbar_ = nullptr;
    empty_state_ = nullptr;
    item_buttons_.clear();
    item_primary_labels_.clear();
    item_secondary_labels_.clear();
    item_accessory_labels_.clear();
    items_.clear();
    selected_index_ = 0;
    title_.clear();
    context_ = {};
}

bool MenuListScreen::handle_input(const InputEvent& input) {
    // First give the top nav bar a chance to handle the input (e.g., hardware back)
    if (top_nav_bar_ && top_nav_bar_->handle_input(input)) {
        return true;
    }
    if (items_.empty()) {
        return false;
    }

    switch (input.key) {
    case InputKey::Up: {
        std::size_t next = selected_index_;
        do {
            next = (next == 0) ? item_buttons_.size() - 1 : next - 1;
        } while (next != selected_index_ && items_[next].is_section_header);
        if (!items_[next].is_section_header) {
            apply_selection(next);
            emit_focus_changed(context_, selected_index_);
        }
        return true;
    }
    case InputKey::Down: {
        std::size_t next = selected_index_;
        do {
            next = (next + 1) % item_buttons_.size();
        } while (next != selected_index_ && items_[next].is_section_header);
        if (!items_[next].is_section_header) {
            apply_selection(next);
            emit_focus_changed(context_, selected_index_);
        }
        return true;
    }
    case InputKey::Press:
        emit_item_selected(context_, selected_index_);
        return true;
    case InputKey::Back:
        return context_.emit_cancel(kMenuComponent);
    case InputKey::Left:
    case InputKey::Right:
        return false;
    }

    return false;
}

bool MenuListScreen::set_data(const PropertyMap& data) {
    const auto context = context_;
    destroy();
    create(context, RouteDescriptor{.route_id = context.route_id, .args = data});
    return true;
}

lv_obj_t* MenuListScreen::item_button(std::size_t index) const noexcept {
    if (index >= item_buttons_.size()) {
        return nullptr;
    }

    return item_buttons_[index];
}

std::vector<MenuListScreen::Item> MenuListScreen::parse_items(const PropertyMap& args) {
    std::vector<Item> items;

    const auto it = args.find(kItemsArg);
    if (it == args.end() || it->second.empty()) {
        return items;
    }

    std::stringstream lines{it->second};
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

        // Section headers: lines starting with "---" become non-selectable dividers
        bool is_header = (parts[0] == "---");
        Item item{.id = parts[0],
                  .label = parts.size() >= 2 && !parts[1].empty() ? parts[1] : (is_header ? "" : parts[0]),
                  .secondary_text = parts.size() >= 3 ? parts[2] : std::string{},
                  .accessory = parts.size() >= 4 ? parts[3] : std::string{},
                  .is_section_header = is_header};
        items.push_back(std::move(item));
    }

    return items;
}

std::size_t MenuListScreen::parse_selected_index(const PropertyMap& args) {
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

std::string MenuListScreen::value_or(const PropertyMap& values, const char* key, const char* fallback) {
    const auto it = values.find(key);
    return it == values.end() ? std::string{fallback} : it->second;
}

void MenuListScreen::apply_selection(std::size_t index) {
    if (item_buttons_.empty()) {
        selected_index_ = 0;
        return;
    }

    // Skip over section headers when selecting
    if (index < items_.size() && items_[index].is_section_header) {
        // Try to find next selectable item
        for (std::size_t i = index + 1; i < items_.size(); ++i) {
            if (!items_[i].is_section_header) { index = i; break; }
        }
    }

    selected_index_ = std::min(index, item_buttons_.size() - 1);
    auto* button = item_buttons_[selected_index_];
    if (button != nullptr && lv_obj_is_valid(button)) {
        lv_obj_scroll_to_view(button, LV_ANIM_OFF);
    }
}

void MenuListScreen::emit_focus_changed(const ScreenContext& context, std::size_t index) const {
    if (index >= items_.size()) {
        return;
    }

    context.emit_action(kFocusAction,
                        kMenuComponent,
                        EventValue{static_cast<std::int64_t>(index)},
                        EventMeta{items_[index].id, std::string{items_[index].label}});
}

void MenuListScreen::emit_item_selected(const ScreenContext& context, std::size_t index) const {
    if (index >= items_.size()) {
        return;
    }

    context.emit_action(kSelectAction,
                        kMenuComponent,
                        EventValue{static_cast<std::int64_t>(index)},
                        EventMeta{items_[index].id, std::string{items_[index].label}});
}

const MenuListScreen::Item* MenuListScreen::find_item(const lv_obj_t* button, std::size_t* index) const noexcept {
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

void MenuListScreen::on_item_event(lv_event_t* event) {
    auto* screen = static_cast<MenuListScreen*>(lv_event_get_user_data(event));
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
        screen->emit_item_selected(screen->context_, index);
    }
}

}  // namespace seedsigner::lvgl
