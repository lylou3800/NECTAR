#include "ui_typography.h"

#include "fonts/nectar_fonts.h"

const lv_font_t *ui_font_display(void)  { return &nectar_display_30; }
const lv_font_t *ui_font_wordmark(void) { return &nectar_display_24; }
const lv_font_t *ui_font_hero(void)     { return &nectar_hero_40; }
const lv_font_t *ui_font_title(void)    { return &nectar_semibold_22; }
const lv_font_t *ui_font_heading(void)  { return &nectar_bold_18; }
const lv_font_t *ui_font_body(void)     { return &nectar_text_17; }
const lv_font_t *ui_font_caption(void)  { return &nectar_text_14; }
