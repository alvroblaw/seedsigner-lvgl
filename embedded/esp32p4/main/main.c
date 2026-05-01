#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

#include "bsp/esp32_p4_wifi6_touch_lcd_4b.h"
#include "bsp/display.h"

static const char *TAG = "sslvgl_p4";

static lv_obj_t *create_menu_row(lv_obj_t *parent,
                                 const char *title,
                                 const char *subtitle,
                                 const char *accessory,
                                 bool selected)
{
    lv_obj_t *button = lv_obj_create(parent);
    lv_obj_remove_style_all(button);
    lv_obj_set_size(button, lv_pct(100), subtitle != NULL ? 96 : 76);
    lv_obj_set_style_radius(button, 18, 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(button, selected ? lv_color_hex(0xFF9F0A) : lv_color_hex(0x2C2C2C), 0);
    lv_obj_set_style_border_width(button, selected ? 2 : 1, 0);
    lv_obj_set_style_border_color(button, selected ? lv_color_hex(0xFFB840) : lv_color_hex(0x414141), 0);
    lv_obj_set_style_pad_left(button, 24, 0);
    lv_obj_set_style_pad_right(button, 24, 0);
    lv_obj_set_style_pad_top(button, 16, 0);
    lv_obj_set_style_pad_bottom(button, 16, 0);
    lv_obj_set_style_pad_row(button, 4, 0);
    lv_obj_set_layout(button, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(button, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(button, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *text_column = lv_obj_create(button);
    lv_obj_remove_style_all(text_column);
    lv_obj_set_flex_grow(text_column, 1);
    lv_obj_set_height(text_column, LV_SIZE_CONTENT);
    lv_obj_set_layout(text_column, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(text_column, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(text_column, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *title_label = lv_label_create(text_column);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_color(title_label, selected ? lv_color_hex(0x000000) : lv_color_hex(0xFCFCFC), 0);
    lv_obj_set_style_text_font(title_label, LV_FONT_DEFAULT, 0);

    if (subtitle != NULL) {
        lv_obj_t *subtitle_label = lv_label_create(text_column);
        lv_label_set_text(subtitle_label, subtitle);
        lv_obj_set_style_text_color(subtitle_label, selected ? lv_color_hex(0x2C2C2C) : lv_color_hex(0x777777), 0);
        lv_obj_set_style_text_font(subtitle_label, LV_FONT_DEFAULT, 0);
    }

    lv_obj_t *accessory_label = lv_label_create(button);
    lv_label_set_text(accessory_label, accessory);
    lv_obj_set_style_text_color(accessory_label, selected ? lv_color_hex(0x000000) : lv_color_hex(0xFCFCFC), 0);
    lv_obj_set_style_text_font(accessory_label, LV_FONT_DEFAULT, 0);

    return button;
}

static void create_bringup_screen(void) {
    ESP_LOGI(TAG, "Creating bring-up screen");

    lv_obj_t *screen = lv_scr_act();
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);

    lv_obj_t *root = lv_obj_create(screen);
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_layout(root, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *topbar = lv_obj_create(root);
    lv_obj_remove_style_all(topbar);
    lv_obj_set_size(topbar, lv_pct(100), 74);
    lv_obj_set_style_bg_color(topbar, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_bg_opa(topbar, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_left(topbar, 20, 0);
    lv_obj_set_style_pad_right(topbar, 20, 0);
    lv_obj_set_style_pad_top(topbar, 12, 0);
    lv_obj_set_style_pad_bottom(topbar, 12, 0);
    lv_obj_set_layout(topbar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(topbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(topbar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *back = lv_btn_create(topbar);
    lv_obj_set_size(back, 42, 42);
    lv_obj_set_style_radius(back, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(back, lv_color_hex(0x2C2C2C), 0);
    lv_obj_set_style_border_width(back, 0, 0);
    lv_obj_t *back_label = lv_label_create(back);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(back_label, lv_color_hex(0xFCFCFC), 0);
    lv_obj_center(back_label);

    lv_obj_t *title = lv_label_create(topbar);
    lv_label_set_text(title, "SeedSigner");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFCFCFC), 0);
    lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
    lv_obj_set_style_pad_left(title, 18, 0);
    lv_obj_set_flex_grow(title, 1);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t *content = lv_obj_create(root);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_style_pad_left(content, 24, 0);
    lv_obj_set_style_pad_right(content, 24, 0);
    lv_obj_set_style_pad_top(content, 24, 0);
    lv_obj_set_style_pad_bottom(content, 24, 0);
    lv_obj_set_style_pad_row(content, 14, 0);
    lv_obj_set_layout(content, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *eyebrow = lv_label_create(content);
    lv_label_set_text(eyebrow, "ESP32-P4 DEMO");
    lv_obj_set_style_text_color(eyebrow, lv_color_hex(0x777777), 0);
    lv_obj_set_style_text_font(eyebrow, LV_FONT_DEFAULT, 0);

    lv_obj_t *headline = lv_label_create(content);
    lv_label_set_text(headline, "Minimal seedsigner-lvgl UI");
    lv_obj_set_style_text_color(headline, lv_color_hex(0xFCFCFC), 0);
    lv_obj_set_style_text_font(headline, LV_FONT_DEFAULT, 0);

    create_menu_row(content, "Scan QR", "Open camera flow demo", LV_SYMBOL_RIGHT, true);
    create_menu_row(content, "Seeds", "View seed tools and backups", LV_SYMBOL_RIGHT, false);
    create_menu_row(content, "Tools", "Utilities and diagnostics", LV_SYMBOL_RIGHT, false);
    create_menu_row(content, "Settings", "Persistent device preferences", LV_SYMBOL_RIGHT, false);

    lv_obj_t *footer_card = lv_obj_create(content);
    lv_obj_remove_style_all(footer_card);
    lv_obj_set_size(footer_card, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_radius(footer_card, 18, 0);
    lv_obj_set_style_bg_color(footer_card, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_bg_opa(footer_card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(footer_card, 1, 0);
    lv_obj_set_style_border_color(footer_card, lv_color_hex(0x414141), 0);
    lv_obj_set_style_pad_all(footer_card, 18, 0);

    lv_obj_t *footer = lv_label_create(footer_card);
    lv_label_set_long_mode(footer, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(footer, lv_pct(100));
    lv_label_set_text(footer,
                      "Display online. PSRAM online. LVGL online.\n"
                      "Next step: route this through the shared seedsigner-lvgl runtime.");
    lv_obj_set_style_text_color(footer, lv_color_hex(0xFF9F0A), 0);
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting ESP32-P4 Waveshare 4B bring-up demo");

    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_H_RES * 40,
        .double_buffer = false,
        .flags = {
            .buff_dma = true,
            .buff_spiram = true,
            .sw_rotate = true,
        },
    };

    lv_display_t *disp = bsp_display_start_with_config(&cfg);
    ESP_LOGI(TAG, "Display start returned: %p", disp);
    assert(disp != NULL);

    /*
     * The BSP re-initializes backlight control during display bring-up.
     * Turning the backlight on before bsp_display_start_with_config() causes
     * a brief visible flash followed by black, because later brightness_init()
     * resets the PWM/GPIO state. So enable brightness only after display init.
     */
    bsp_display_backlight_on();
    bsp_display_brightness_set(100);
    ESP_LOGI(TAG, "Backlight configured after display init");

    bool locked = bsp_display_lock(1000);
    ESP_LOGI(TAG, "Display lock: %s", locked ? "ok" : "failed");
    if (!locked) {
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            ESP_LOGE(TAG, "Unable to obtain LVGL lock");
        }
    }

    create_bringup_screen();
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    ESP_LOGI(TAG, "Bring-up screen rendered");
    bsp_display_unlock();
    ESP_LOGI(TAG, "Display unlocked");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG, "bring-up alive");
    }
}
