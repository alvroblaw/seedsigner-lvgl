#ifndef LV_CONF_H
#define LV_CONF_H

/*
 * Project-owned LVGL configuration for host and future embedded builds.
 *
 * Keep this file in-repo so contributors do not need to copy or generate
 * a local lv_conf.h before configuring the project.
 *
 * We intentionally keep overrides light for now and rely on LVGL's
 * lv_conf_internal.h defaults for everything not specified here.
 */

/* Match the current headless host framebuffer setup. */
#define LV_COLOR_DEPTH 16

/* Default to the built-in allocator until platform-specific memory plumbing exists. */
#define LV_MEM_CUSTOM 0

/* Keep logging disabled in the bootstrap phase unless a target opts in later. */
#define LV_USE_LOG 0

/* Enable QR code library */
#define LV_USE_QRCODE 1

#endif /* LV_CONF_H */
