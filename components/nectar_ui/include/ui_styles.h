#pragma once

#include "lvgl.h"
#include "ui_palette.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_styles_init(void);
lv_style_t *ui_style_screen(void);
lv_style_t *ui_style_card(void);
lv_style_t *ui_style_card_highlight(void);
lv_style_t *ui_style_card_inset(void);
lv_style_t *ui_style_button_primary(void);
lv_style_t *ui_style_button_primary_pressed(void);
lv_style_t *ui_style_button_secondary(void);
lv_style_t *ui_style_button_secondary_pressed(void);
lv_style_t *ui_style_button_disabled(void);
lv_style_t *ui_style_title(void);
lv_style_t *ui_style_heading(void);
lv_style_t *ui_style_body(void);
lv_style_t *ui_style_caption(void);
lv_style_t *ui_style_overline(void);
lv_style_t *ui_style_banner(void);
lv_style_t *ui_style_banner_warning(void);
lv_style_t *ui_style_banner_error(void);
lv_style_t *ui_style_badge(void);
lv_style_t *ui_style_badge_alert(void);

#ifdef __cplusplus
}
#endif