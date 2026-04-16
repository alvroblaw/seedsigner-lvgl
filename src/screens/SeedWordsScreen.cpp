#include "seedsigner_lvgl/screens/SeedWordsScreen.hpp"
#include "seedsigner_lvgl/visual/DisplayProfile.hpp"
#include "seedsigner_lvgl/visual/SeedSignerTheme.hpp"

#include "seedsigner_lvgl/components/TopNavBar.hpp"

#include <lvgl.h>
#include <algorithm>
#include <cstdio>
#include <string>

namespace seedsigner::lvgl {
namespace {

constexpr const char* kPageChangedAction = "page_changed";
constexpr const char* kBackRequestedAction = "back_requested";
constexpr const char* kWordSelectedAction = "word_selected";
constexpr const char* kSeedWordsComponent = "seed_words_screen";

constexpr const char* kDefaultWarningText = "Do not digitize these words. Write them down and store offline.";
constexpr const char* kDefaultTitle = "Seed Words";

// SeedSigner-style: 2-column grid, each chip shows "N. word"
constexpr int kColumns = 2;
constexpr int kChipH = 28;          // compact row height
constexpr int kGridGap = 4;         // gap between chips

int chip_width_px() {
    // Subtract padding and gap from screen width
    // SCREEN_PADDING on each side + gap between columns
    return (profile::active().width - 2 * theme::spacing::SCREEN_PADDING - kGridGap) / kColumns;
}

const char* value_or(const PropertyMap& values, const char* key, const char* fallback) {
    const auto it = values.find(key);
    return it == values.end() ? fallback : it->second.c_str();
}

}  // namespace

void SeedWordsScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    context_ = context;
    params_ = parse_seed_words_params(route.args);

    // Use profile words_per_page if not overridden
    if (params_.words_per_page <= 0) {
        params_.words_per_page = profile::active().words_per_page;
    }

    // Calculate total pages
    total_pages_ = (static_cast<int>(params_.words.size()) + params_.words_per_page - 1) / params_.words_per_page;
    current_page_ = 0;

    // Initialize styles
    if (!styles_initialized_) {
        // Normal chip: dark surface, subtle border
        lv_style_init(&chip_style_);
        lv_style_set_radius(&chip_style_, theme::spacing::CHIP_RADIUS);
        lv_style_set_bg_opa(&chip_style_, LV_OPA_COVER);
        lv_style_set_bg_color(&chip_style_, theme::active_theme().SURFACE_MEDIUM);
        lv_style_set_border_width(&chip_style_, 1);
        lv_style_set_border_color(&chip_style_, theme::active_theme().BORDER);
        lv_style_set_pad_ver(&chip_style_, 4);
        lv_style_set_pad_hor(&chip_style_, 6);
        lv_style_set_text_color(&chip_style_, theme::active_theme().TEXT_PRIMARY);
        lv_style_set_text_font(&chip_style_, theme::typography::BODY);

        // Warning chip (unused for now, kept for future highlight)
        lv_style_init(&warning_chip_style_);
        lv_style_set_radius(&warning_chip_style_, theme::spacing::CHIP_RADIUS);
        lv_style_set_bg_opa(&warning_chip_style_, LV_OPA_20);
        lv_style_set_bg_color(&warning_chip_style_, theme::active_theme().PRIMARY);
        lv_style_set_border_width(&warning_chip_style_, 2);
        lv_style_set_border_color(&warning_chip_style_, theme::active_theme().PRIMARY);
        lv_style_set_pad_ver(&warning_chip_style_, 4);
        lv_style_set_pad_hor(&warning_chip_style_, 6);
        lv_style_set_text_color(&warning_chip_style_, theme::active_theme().TEXT_PRIMARY);
        lv_style_set_text_font(&warning_chip_style_, theme::typography::BODY);

        styles_initialized_ = true;
    }

    // Root container
    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(container_, theme::active_theme().SURFACE_DARK, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);

    // Top navigation bar
    top_nav_bar_ = std::make_unique<TopNavBar>(context);
    TopNavBarConfig nav_config;
    nav_config.title = params_.title.value_or(kDefaultTitle);
    nav_config.show_back = true;
    top_nav_bar_->set_config(nav_config);
    top_nav_bar_->attach(container_);

    // Content container
    content_container_ = lv_obj_create(container_);
    lv_obj_set_size(content_container_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(content_container_, 1);
    lv_obj_set_flex_flow(content_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(content_container_, theme::spacing::SCREEN_PADDING, 0);
    lv_obj_set_style_pad_row(content_container_, 4, 0);
    lv_obj_set_style_bg_opa(content_container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content_container_, 0, 0);
    lv_obj_clear_flag(content_container_, LV_OBJ_FLAG_SCROLLABLE);

    // --- Warning banner ---
    warning_label_ = lv_label_create(content_container_);
    lv_obj_set_width(warning_label_, lv_pct(100));
    lv_label_set_long_mode(warning_label_, LV_LABEL_LONG_WRAP);
    lv_label_set_text(warning_label_, params_.warning_text.value_or(kDefaultWarningText).c_str());
    lv_obj_set_style_text_align(warning_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(warning_label_, theme::active_theme().WARNING, 0);
    lv_obj_set_style_text_font(warning_label_, theme::typography::CAPTION, 0);
    lv_obj_set_style_pad_bottom(warning_label_, 6, 0);

    // --- Page indicator ---
    page_label_ = lv_label_create(content_container_);
    lv_label_set_text_fmt(page_label_, "%d / %d", current_page_ + 1, total_pages_);
    lv_obj_set_style_text_color(page_label_, theme::active_theme().TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(page_label_, theme::typography::CAPTION, 0);
    lv_obj_set_style_pad_bottom(page_label_, 4, 0);

    // --- Word grid container ---
    words_container_ = lv_obj_create(content_container_);
    lv_obj_set_size(words_container_, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(words_container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(words_container_, 0, 0);
    lv_obj_set_style_pad_all(words_container_, 0, 0);
    lv_obj_set_flex_flow(words_container_, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(words_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(words_container_, kGridGap, 0);
    lv_obj_set_style_pad_row(words_container_, kGridGap, 0);
    lv_obj_clear_flag(words_container_, LV_OBJ_FLAG_SCROLLABLE);

    // Populate first page
    update_page();

    // --- Nav hint at bottom ---
    if (total_pages_ > 1) {
        lv_obj_t* hint = lv_label_create(content_container_);
        lv_label_set_text(hint, LV_SYMBOL_LEFT " Prev             Next " LV_SYMBOL_RIGHT);
        lv_obj_set_style_text_color(hint, theme::active_theme().TEXT_SECONDARY, 0);
        lv_obj_set_style_text_font(hint, theme::typography::CAPTION, 0);
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
        case InputKey::Back:
            emit_back_requested();
            return true;
        case InputKey::Press:
        case InputKey::Up:
        case InputKey::Down:
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

    // Update page indicator
    if (page_label_) {
        lv_label_set_text_fmt(page_label_, "%d / %d", current_page_ + 1, total_pages_);
    }

    // Calculate word range for this page
    int start = current_page_ * params_.words_per_page;
    int end = std::min(start + params_.words_per_page, static_cast<int>(params_.words.size()));

    int cw = chip_width_px();

    for (int i = start; i < end; ++i) {
        create_word_chip(words_container_, i, params_.words[i], cw);
    }
}

void SeedWordsScreen::create_word_chip(lv_obj_t* parent, int index, const std::string& word, int width) {
    lv_obj_t* chip = lv_obj_create(parent);
    lv_obj_set_size(chip, width, kChipH);
    lv_obj_add_style(chip, &chip_style_, 0);
    lv_obj_clear_flag(chip, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(chip, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(chip, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(chip, 4, 0);

    // Word number (dimmed, fixed width)
    lv_obj_t* num = lv_label_create(chip);
    char num_buf[8];
    std::snprintf(num_buf, sizeof(num_buf), "%2d.", index + 1);
    lv_label_set_text(num, num_buf);
    lv_obj_set_style_text_color(num, theme::active_theme().TEXT_SECONDARY, 0);
    lv_obj_set_style_text_font(num, theme::typography::CAPTION, 0);
    lv_obj_set_width(num, 28);

    // Word text
    lv_obj_t* lbl = lv_label_create(chip);
    lv_label_set_text(lbl, word.c_str());
    lv_obj_set_style_text_font(lbl, theme::typography::BODY, 0);
    lv_obj_set_flex_grow(lbl, 1);

    word_chips_.push_back(chip);
}

void SeedWordsScreen::apply_warning_styling(lv_obj_t* obj) {
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
