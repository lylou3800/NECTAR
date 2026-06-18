#pragma once

#include "lvgl.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

lv_color_t ui_color_background(void);
lv_color_t ui_color_background_alt(void);
lv_color_t ui_color_surface(void);
lv_color_t ui_color_surface_alt(void);
lv_color_t ui_color_surface_highlight(void);
lv_color_t ui_color_surface_overlay(void);
lv_color_t ui_color_accent(void);
lv_color_t ui_color_accent_soft(void);
lv_color_t ui_color_accent_secondary(void);
lv_color_t ui_color_accent_glow(void);
lv_color_t ui_color_text_primary(void);
lv_color_t ui_color_text_secondary(void);
lv_color_t ui_color_text_muted(void);
lv_color_t ui_color_text_ink(void);
lv_color_t ui_color_success(void);
lv_color_t ui_color_warning(void);
lv_color_t ui_color_error(void);
lv_color_t ui_color_info(void);
lv_color_t ui_color_disabled(void);
lv_color_t ui_color_line(void);
lv_color_t ui_color_line_soft(void);
lv_color_t ui_color_family(size_t index);

#ifdef __cplusplus
}
#endif