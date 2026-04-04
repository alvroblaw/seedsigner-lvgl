#include "seedsigner_lvgl/screens/PlaceholderScreen.hpp"

namespace seedsigner::lvgl {

namespace {

const char* value_or(const PropertyMap& values, const char* key, const char* fallback) {
    const auto it = values.find(key);
    return it == values.end() ? fallback : it->second.c_str();
}

}  // namespace

void PlaceholderScreen::create(const ScreenContext& context, const RouteDescriptor& route) {
    container_ = lv_obj_create(context.root);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container_, 12, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    auto* title = lv_label_create(container_);
    lv_obj_set_width(title, lv_pct(100));
    lv_label_set_text(title, value_or(route.args, "title", "SeedSigner LVGL"));

    auto* body = lv_label_create(container_);
    lv_obj_set_width(body, lv_pct(100));
    lv_label_set_long_mode(body, LV_LABEL_LONG_WRAP);
    lv_label_set_text(body, value_or(route.args, "body", "Host-simulated runtime skeleton booted."));
}

void PlaceholderScreen::destroy() {
    if (container_ != nullptr) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
}

}  // namespace seedsigner::lvgl
