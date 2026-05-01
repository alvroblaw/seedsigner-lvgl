#include <memory>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bsp/esp32_p4_wifi6_touch_lcd_4b.h"
#include "bsp/display.h"

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"
#include "seedsigner_lvgl/runtime/Event.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"
#include "seedsigner_lvgl/screens/MenuListScreen.hpp"

using namespace seedsigner::lvgl;

namespace {

constexpr const char *TAG = "sslvgl_p4";

std::unique_ptr<MenuListScreen> g_menu_screen;

bool emit_event(UiEvent event)
{
    if (event.action_id.has_value()) {
        ESP_LOGI(TAG,
                 "UI event: type=%d action=%s component=%s",
                 static_cast<int>(event.type),
                 event.action_id->c_str(),
                 event.component_id.has_value() ? event.component_id->c_str() : "-");
    }
    return true;
}

std::uint64_t now_ms()
{
    return static_cast<std::uint64_t>(xTaskGetTickCount()) * portTICK_PERIOD_MS;
}

void create_demo_ui()
{
    ScreenContext context{
        .root = lv_scr_act(),
        .route_id = RouteId{"demo.menu"},
        .screen_token = 1,
        .emit_event = emit_event,
        .now_ms = now_ms,
    };

    RouteDescriptor route{
        .route_id = RouteId{"demo.menu"},
        .args = {
            {"title", "SeedSigner"},
            {"selected_index", "0"},
            {"items",
             "scan|Scan QR|Open camera flow demo|chevron\n"
             "seeds|Seeds|View seed tools and backups|chevron\n"
             "tools|Tools|Utilities and diagnostics|chevron\n"
             "settings|Settings|Persistent device preferences|chevron"},
        },
    };

    g_menu_screen = std::make_unique<MenuListScreen>();
    g_menu_screen->create(context, route);
    g_menu_screen->on_activate();
}

} // namespace

extern "C" void app_main(void)
{
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
    assert(disp != nullptr);

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

    create_demo_ui();
    lv_obj_invalidate(lv_scr_act());
    bsp_display_unlock();
    ESP_LOGI(TAG, "SeedSigner demo UI queued for LVGL render");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG, "bring-up alive");
    }
}
