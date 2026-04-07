#pragma once

#include <string>
#include <vector>

#include "seedsigner_lvgl/contracts/SeedWordsContract.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

class SeedWordsScreen : public Screen {
public:
    void create(const ScreenContext& context, const RouteDescriptor& route) override;
    void destroy() override;
    bool handle_input(const InputEvent& input) override;

private:
    void update_page();
    void create_word_chip(lv_obj_t* parent, int index, const std::string& word);
    void apply_warning_styling(lv_obj_t* obj);
    void emit_page_changed();
    void emit_back_requested();
    void emit_word_selected(int index, const std::string& word);

    ScreenContext context_{};
    lv_obj_t* container_{nullptr};
    lv_obj_t* title_label_{nullptr};
    lv_obj_t* page_label_{nullptr};
    lv_obj_t* warning_label_{nullptr};
    lv_obj_t* words_container_{nullptr};
    lv_obj_t* prev_button_{nullptr};
    lv_obj_t* next_button_{nullptr};
    lv_style_t chip_style_{};
    lv_style_t warning_chip_style_{};
    bool styles_initialized_{false};

    SeedWordsParams params_{};
    int current_page_{0};
    int total_pages_{0};
    std::vector<lv_obj_t*> word_chips_{};
};

}  // namespace seedsigner::lvgl