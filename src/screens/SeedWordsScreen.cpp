#include "seedsigner_lvgl/screens/SeedWordsScreen.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"

#include "seedsigner_lvgl/components/TopNavBar.hpp"

#include <lvgl.h>
#include <algorithm>
#include <string>

namespace seedsigner::lvgl {
namespace {

constexpr const char* kPageChangedAction = "page_changed";
constexpr const char* kBackRequestedAction = "back_requested";
constexpr const char* kWordSelectedAction = "word_selected";
constexpr const char* kSeedWordsComponent = "seed_words_screen";

constexpr const char* kDefaultWarningText = "Never digitize these words. Store this offline.";
constexpr const char* kDefaultTitle = "Seed Words";

constexpr int kChipWidth = 100;  // adjust based on screen size
constexpr int kChipHeight = 40;
constexpr int kChipMargin = 6;

const char* value_or(const PropertyMap& values, const char* key, const char* fallback) {
    const auto it = values.find(key);
    return it == values.end() ? fallback : it->second.c_str();
}

}  // namespace

void SeedWordsScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    params_ = parse_seed_words_params(route.args);
    
    // Calculate total pages
    total_pages_ = (params_.words.size() + params_.words_per_page - 1) / params_.words_per_page;
    current_page_ = 0;
    
    // Initialize styles if needed
    if (!styles_initialized_) {
        lv_style_init(&chip_style_);
        lv_style_set_radius(&chip_style_, 8);
        lv_style_set_bg_opa(&chip_style_, LV_OPA_20);
        lv_style_set_bg_color(&chip_style_, seedsigner::lvgl::theme::colors::SURFACE_MEDIUM);
        lv_style_set_border_width(&chip_style_, 1);
        lv_style_set_border_color(&chip_style_, seedsigner::lvgl::theme::colors::BORDER);
        lv_style_set_pad_all(&chip_style_, 8);
        lv_style_set_text_color(&chip_style_, seedsigner::lvgl::theme::colors::TEXT_PRIMARY);
        
        lv_style_init(&warning_chip_style_);
        lv_style_set_radius(&warning_chip_style_, 8);
        lv_style_set_bg_opa(&warning_chip_style_, LV_OPA_20);
        lv_style_set_bg_color(&warning_chip_style_, seedsigner::lvgl::theme::colors::PRIMARY);
        lv_style_set_border_width(&warning_chip_style_, 2);
        lv_style_set_border_color(&warning_chip_style_, seedsigner::lvgl::theme::colors::PRIMARY);
        lv_style_set_pad_all(&warning_chip_style_, 8);
        lv_style_set_text_color(&warning_chip_style_, seedsigner::lvgl::theme::colors::TEXT_PRIMARY);
        styles_initialized_ = true;
    }
    
    // Create container
    // Root container: column with top nav bar and content area
    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Top navigation bar
    top_nav_bar_ = std::make_unique<TopNavBar>(context);
    TopNavBarConfig nav_config;
    nav_config.title = params_.title.value_or(kDefaultTitle);
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
    lv_obj_set_flex_align(content_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(content_container_, 12, 0);
    
    // Page indicator
    page_label_ = lv_label_create(content_container_);
    lv_obj_set_width(page_label_, lv_pct(100));
    lv_label_set_text_fmt(page_label_, "Page %d/%d", current_page_ + 1, total_pages_);
    lv_obj_set_style_text_align(page_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_bottom(page_label_, 12, 0);
    
    // Warning text
    const std::string warning = params_.warning_text.value_or(kDefaultWarningText);
    warning_label_ = lv_label_create(content_container_);
    lv_obj_set_width(warning_label_, lv_pct(100));
    lv_label_set_long_mode(warning_label_, LV_LABEL_LONG_WRAP);
    lv_label_set_text(warning_label_, warning.c_str());
    lv_obj_set_style_text_align(warning_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(warning_label_, lv_palette_main(LV_PALETTE_ORANGE), 0);
    lv_obj_set_style_pad_bottom(warning_label_, 16, 0);
    
    // Words container (grid)
    words_container_ = lv_obj_create(content_container_);
    lv_obj_set_size(words_container_, lv_pct(100), lv_pct(60));
    lv_obj_set_flex_flow(words_container_, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(words_container_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(words_container_, 0, 0);
    lv_obj_set_style_pad_gap(words_container_, kChipMargin, 0);
    
    // Create chips for current page
    update_page();
    
    // Navigation buttons (prev/next) if multiple pages
    if (total_pages_ > 1) {
        lv_obj_t* button_row = lv_obj_create(content_container_);
        lv_obj_set_size(button_row, lv_pct(100), 48);
        lv_obj_set_flex_flow(button_row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(button_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(button_row, 0, 0);
        
        prev_button_ = lv_btn_create(button_row);
        lv_obj_set_size(prev_button_, 80, 40);
        lv_obj_set_style_radius(prev_button_, 8, 0);
        lv_obj_add_event_cb(prev_button_, [](lv_event_t* e) {
            auto* screen = static_cast<SeedWordsScreen*>(lv_event_get_user_data(e));
            if (screen != nullptr && screen->current_page_ > 0) {
                screen->current_page_--;
                screen->update_page();
                screen->emit_page_changed();
            }
        }, LV_EVENT_CLICKED, this);
        lv_obj_t* prev_label = lv_label_create(prev_button_);
        lv_label_set_text(prev_label, LV_SYMBOL_LEFT " Prev");
        lv_obj_center(prev_label);
        
        next_button_ = lv_btn_create(button_row);
        lv_obj_set_size(next_button_, 80, 40);
        lv_obj_set_style_radius(next_button_, 8, 0);
        lv_obj_add_event_cb(next_button_, [](lv_event_t* e) {
            auto* screen = static_cast<SeedWordsScreen*>(lv_event_get_user_data(e));
            if (screen != nullptr && screen->current_page_ < screen->total_pages_ - 1) {
                screen->current_page_++;
                screen->update_page();
                screen->emit_page_changed();
            }
        }, LV_EVENT_CLICKED, this);
        lv_obj_t* next_label = lv_label_create(next_button_);
        lv_label_set_text(next_label, "Next " LV_SYMBOL_RIGHT);
        lv_obj_center(next_label);
    }
}

void SeedWordsScreen::destroy() {
    top_nav_bar_.reset();
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
    content_container_ = nullptr;
    title_label_ = nullptr;
    page_label_ = nullptr;
    warning_label_ = nullptr;
    words_container_ = nullptr;
    prev_button_ = nullptr;
    next_button_ = nullptr;
    word_chips_.clear();
    context_ = {};
}

bool SeedWordsScreen::handle_input(const InputEvent& input) {
    // Let TopNavBar handle input first (e.g., hardware back button)
    if (top_nav_bar_ && top_nav_bar_->handle_input(input)) {
        return true;
    }

    switch (input.key) {
        case InputKey::Left:
            if (current_page_ > 0) {
                current_page_--;
                update_page();
                emit_page_changed();
                return true;
            }
            break;
        case InputKey::Right:
            if (current_page_ < total_pages_ - 1) {
                current_page_++;
                update_page();
                emit_page_changed();
                return true;
            }
            break;
        case InputKey::Press:
            // Optionally emit word_selected for focused chip (not implemented yet)
            // For now, treat as "select" maybe do nothing.
            return false;
        case InputKey::Back:
            emit_back_requested();
            return true;
        case InputKey::Up:
        case InputKey::Down:
            // Could be used to navigate between chips vertically (future)
            return false;

    }
    return false;
}

void SeedWordsScreen::update_page() {
    // Clear existing chips
    for (lv_obj_t* chip : word_chips_) {
        lv_obj_del(chip);
    }
    word_chips_.clear();
    
    // Update page label
    if (page_label_) {
        lv_label_set_text_fmt(page_label_, "Page %d/%d", current_page_ + 1, total_pages_);
    }
    
    // Calculate start and end indices
    int start = current_page_ * params_.words_per_page;
    int end = std::min(start + params_.words_per_page, static_cast<int>(params_.words.size()));
    
    // Create chips for words on this page
    for (int i = start; i < end; ++i) {
        create_word_chip(words_container_, i, params_.words[i]);
    }
}

void SeedWordsScreen::create_word_chip(lv_obj_t* parent, int index, const std::string& word) {
    lv_obj_t* chip = lv_btn_create(parent);
    lv_obj_set_size(chip, kChipWidth, kChipHeight);
    lv_obj_add_style(chip, &chip_style_, 0);
    apply_warning_styling(chip); // Add warning styling
    
    // Add click event for word selection
    lv_obj_add_event_cb(chip, [](lv_event_t* e) {
        auto* screen = static_cast<SeedWordsScreen*>(lv_event_get_user_data(e));
        if (screen != nullptr) {
            // Extract index from user_data stored in chip? For simplicity, we'll pass index via event user_data.
            // We'll need to store index in chip's user_data. For now, we'll skip.
        }
    }, LV_EVENT_CLICKED, this);
    
    // Create label: index + word
    lv_obj_t* label = lv_label_create(chip);
    lv_label_set_text_fmt(label, "%d. %s", index + 1, word.c_str());
    lv_obj_center(label);
    
    word_chips_.push_back(chip);
}

void SeedWordsScreen::apply_warning_styling(lv_obj_t* obj) {
    // Apply warning chip style (orange/red edges)
    lv_obj_add_style(obj, &warning_chip_style_, 0);
}

void SeedWordsScreen::emit_page_changed() {
    SeedWordsEvent event;
    event.type = SeedWordsEvent::Type::PageChanged;
    event.page_index = current_page_;
    event.total_pages = total_pages_;
    std::string encoded = encode_seed_words_event(event);
    context_.emit_action(kPageChangedAction, kSeedWordsComponent, EventValue{encoded});
}

void SeedWordsScreen::emit_back_requested() {
    SeedWordsEvent event;
    event.type = SeedWordsEvent::Type::BackRequested;
    std::string encoded = encode_seed_words_event(event);
    context_.emit_action(kBackRequestedAction, kSeedWordsComponent, EventValue{encoded});
}

void SeedWordsScreen::emit_word_selected(int index, const std::string& word) {
    SeedWordsEvent event;
    event.type = SeedWordsEvent::Type::WordSelected;
    event.word_index = index;
    event.word = word;
    std::string encoded = encode_seed_words_event(event);
    context_.emit_action(kWordSelectedAction, kSeedWordsComponent, EventValue{encoded});
}

}  // namespace seedsigner::lvgl