# ESP32-P4 Waveshare 4B bring-up

## Goal

Reproducible first bring-up for `seedsigner-lvgl` on:

- **Board:** Waveshare `ESP32-P4-WIFI6-Touch-LCD-4B`
- **Target:** `esp32p4`
- **Scope of this phase:** panel online, LVGL online, backlight online, touch stack available through BSP, simple screen rendered from an ESP-IDF app

This phase does **not** yet wire the full `seedsigner_lvgl` C++ runtime into the board-specific display path. It establishes the real hardware base needed for the next step.

## Why this setup exists

The board already has a maintained ESP-IDF BSP/component path that exposes:

- `waveshare/esp32_p4_wifi6_touch_lcd_4b`
- `waveshare/esp_lcd_st7703`
- `espressif/esp_lvgl_port`
- `espressif/esp_lcd_touch_gt911`

Using those first is lower risk than writing raw LCD/touch bring-up from scratch.

## Repo layout

Embedded P4 bring-up project lives in:

```text
embedded/esp32p4/
```

Key files:

- `embedded/esp32p4/idf_component.yml`
- `embedded/esp32p4/sdkconfig.defaults`
- `embedded/esp32p4/main/main.c`

## Prerequisites

- ESP-IDF **5.4.4** validated for this project
- `idf.py` available in shell
- board connected over the UART/flash USB port

Preferred reproducible path from repo root:

```bash
cd third_party/esp-idf
./install.sh esp32p4
cd ../..
. ./third_party/esp-idf/export.sh
```

Check environment:

```bash
idf.py --version
python --version
```

## Build

From repo root:

```bash
cd embedded/esp32p4
idf.py set-target esp32p4
idf.py build
```

On first build, IDF downloads managed components declared in `idf_component.yml`.

This repo also carries local component overrides under `embedded/esp32p4/components/` for currently-needed compatibility fixes against the validated BSP stack.

## Flash

Find your serial device first, then:

```bash
cd embedded/esp32p4
idf.py -p /dev/ttyACM0 flash monitor
```

Adjust port as needed.

## Expected result

If bring-up works, board should:

- turn on backlight
- initialize panel through BSP
- initialize LVGL through `esp_lvgl_port`
- render a black screen with:
  - `seedsigner-lvgl`
  - `ESP32-P4 Waveshare 4B bring-up`
  - status text card
- emit periodic `bring-up alive` logs in monitor

## What this proves

This proves embedded base path is real:

- ESP-IDF project works
- Waveshare BSP resolves and builds
- panel path works on real hardware
- LVGL render path works on real hardware
- project now has reproducible board-specific starting point

## What comes next

Next phase on this same branch should add platform glue so `seedsigner_lvgl` runtime can use embedded LVGL/display instead of host/headless display plumbing.

Recommended next increments:

1. add a thin `EspP4Display` / platform adapter layer
2. expose touch input as runtime input events
3. boot a minimal `MenuListScreen` through runtime, not direct LVGL labels
4. document memory/buffer tradeoffs on real hardware

## Notes about this board

For this specific Waveshare board, current assumption is:

- display path comes from BSP-managed ST7703 stack
- touch path comes from BSP-managed GT911 stack
- backlight control is exposed by BSP helper APIs

Validated stack for reproducible host build:

- `third_party/esp-idf`: **v5.4.4**
- `waveshare/esp32_p4_wifi6_touch_lcd_4b`: `1.0.1`
- `waveshare/esp_lcd_st7703`: `1.0.5`
- `espressif/esp_lvgl_port`: `2.7.2`
- `espressif/usb`: `1.4.0`
- `lvgl/lvgl`: `9.5.0`

Local overrides currently patch BSP/component compatibility gaps for this stack:

- `embedded/esp32p4/components/esp_codec_dev`
- `embedded/esp32p4/components/esp_lcd_st7703`
- `embedded/esp32p4/components/esp32_p4_wifi6_touch_lcd_4b`

If future Waveshare BSP versions change APIs, keep this document updated with exact component versions that were validated.

## Troubleshooting

### `idf.py: command not found`
ESP-IDF environment not exported. Source `export.sh` from your IDF install.

### managed component download fails
Network issue or registry resolution problem. Re-run `idf.py build` after environment/network fixed.

### build succeeds but screen stays dark
Check:

- correct USB/flash port and successful flash
- board power path
- BSP version lock in `idf_component.yml`
- backlight helper calls reached in logs

### touch not used yet
Expected in this phase. BSP touch dependency is present, but runtime input integration is next step, not part of first panel bring-up.
