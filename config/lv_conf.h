#ifndef LV_CONF_H
#define LV_CONF_H

/*
 * Project-owned LVGL v9 configuration for host and embedded builds.
 *
 * Keep this file in-repo so contributors do not need to copy or generate
 * a local lv_conf.h before configuring the project.
 *
 * LVGL v9 restructured many config options; this file follows the v9.3.0
 * lv_conf_template.h layout.
 */

/* Color depth: 16 = RGB565 */
#define LV_COLOR_DEPTH 16

/* Use LVGL's built-in stdlib wrappers (v9 replaces LV_MEM_CUSTOM) */
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_BUILTIN

/* Keep logging disabled unless a target opts in */
#define LV_USE_LOG 0

/* Software renderer (required for headless host builds) */
#define LV_USE_DRAW_SW 1

/* Enable QR code library */
#define LV_USE_QRCODE 1

/* Enable snapshot for screen capture / testing */
#define LV_USE_SNAPSHOT 1

/* Enable Montserrat fonts used by SeedSigner theme */
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1

#endif /* LV_CONF_H */
