#include "seedsigner_lvgl/screens/MenuListScreen.hpp"

#include <algorithm>
#include <sstream>
#include <string>

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

}  // namespace

void MenuListScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    title_ = value_or(route.args, kTitleArg, "Menu");
    items_ = parse_items(route.args);
    selected_index_ = parse_selected_index(route.args);
    item_buttons_.clear();

    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 12, 0);
    lv_obj_set_style_pad_row(container_, 8, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    auto* title = lv_label_create(container_);
    lv_obj_set_width(title, lv_pct(100));
    lv_label_set_text(title, title_.c_str());

    list_ = lv_list_create(container_);
    lv_obj_set_size(list_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(list_, 1);

    if (items_.empty()) {
        empty_state_ = lv_label_create(list_);
        lv_obj_set_width(empty_state_, lv_pct(100));
        lv_label_set_long_mode(empty_state_, LV_LABEL_LONG_WRAP);
        lv_label_set_text(empty_state_, "No menu items provided.");
        return;
    }

    if (selected_index_ >= items_.size()) {
        selected_index_ = 0;
    }

    for (std::size_t index = 0; index < items_.size(); ++index) {
        const auto& item = items_[index];
        auto* button = lv_list_add_btn(list_, nullptr, item.label.c_str());
        item_buttons_.push_back(button);
        lv_obj_add_event_cb(button, &MenuListScreen::on_item_event, LV_EVENT_FOCUSED, this);
        lv_obj_add_event_cb(button, &MenuListScreen::on_item_event, LV_EVENT_CLICKED, this);
    }

    apply_selection(selected_index_);
}

void MenuListScreen::destroy() {
    if (container_ != nullptr) {
        lv_obj_del(container_);
    }

    container_ = nullptr;
    list_ = nullptr;
    empty_state_ = nullptr;
    item_buttons_.clear();
    items_.clear();
    selected_index_ = 0;
    title_.clear();
    context_ = {};
}

bool MenuListScreen::handle_input(const InputEvent& input) {
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
    destroy();
    create(context_, RouteDescriptor{.route_id = context_.route_id, .args = data});
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

        const auto separator = line.find('|');
        if (separator == std::string::npos) {
            items.push_back(Item{.id = line, .label = line});
            continue;
        }

        auto id = trim(line.substr(0, separator));
        auto label = trim(line.substr(separator + 1));
        if (id.empty()) {
            continue;
        }
        if (label.empty()) {
            label = id;
        }

        items.push_back(Item{.id = std::move(id), .label = std::move(label)});
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

    selected_index_ = std::min(index, item_buttons_.size() - 1);
    for (std::size_t button_index = 0; button_index < item_buttons_.size(); ++button_index) {
        auto* button = item_buttons_[button_index];
        const bool is_selected = button_index == selected_index_;
        lv_obj_set_style_bg_opa(button, is_selected ? LV_OPA_20 : LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(button, is_selected ? 2 : 0, LV_PART_MAIN);
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
