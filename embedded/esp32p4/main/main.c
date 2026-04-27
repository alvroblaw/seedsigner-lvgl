#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

#include "bsp/esp32_p4_wifi6_touch_lcd_4b.h"
#include "bsp/display.h"

static const char *TAG = "sslvgl_p4";

static void create_bringup_screen(void) {
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "seedsigner-lvgl");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFCFCFC), 0);
    lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

    lv_obj_t *subtitle = lv_label_create(screen);
    lv_label_set_text(subtitle, "ESP32-P4 Waveshare 4B bring-up");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xB8B8B8), 0);
    lv_obj_set_style_text_font(subtitle, LV_FONT_DEFAULT, 0);
    lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);

    lv_obj_t *card = lv_obj_create(screen);
    lv_obj_remove_style_all(card);
    lv_obj_set_size(card, 620, 220);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_border_width(card, 2, 0);
    lv_obj_set_style_radius(card, 18, 0);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 24);

    lv_obj_t *body = lv_label_create(card);
    lv_label_set_text(body,
                      "Display: online\n"
                      "Touch: BSP-managed\n"
                      "Backlight: enabled\n"
                      "Next: integrate seedsigner runtime platform layer");
    lv_obj_set_style_text_color(body, lv_color_hex(0xFCFCFC), 0);
    lv_obj_set_style_text_font(body, LV_FONT_DEFAULT, 0);
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 24, 24);

    lv_obj_t *footer = lv_label_create(screen);
    lv_label_set_text(footer, "If this screen renders, panel + LVGL path works.");
    lv_obj_set_style_text_color(footer, lv_color_hex(0xFF9F0A), 0);
    lv_obj_set_style_text_font(footer, LV_FONT_DEFAULT, 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -24);
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting ESP32-P4 Waveshare 4B bring-up demo");

    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_H_RES * 80,
        .double_buffer = true,
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,
        },
    };

    bsp_display_brightness_init();
    bsp_display_backlight_on();
    bsp_display_brightness_set(100);

    bsp_display_start_with_config(&cfg);
    bsp_display_lock(0);
    create_bringup_screen();
    bsp_display_unlock();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG, "bring-up alive");
    }
}
