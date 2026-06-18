#include "screen_boot.h"

#include "ui_chrome.h"
#include "ui_metrics.h"
#include "ui_state_model.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_typography.h"

static lv_obj_t *s_loading_value;

void screen_boot_create(lv_obj_t *screen)
{
    lv_obj_t *content = lv_obj_create(screen);
    lv_obj_t *eyebrow;
    lv_obj_t *brand;
    lv_obj_t *subtitle;
    lv_obj_t *loading;
    lv_obj_t *hint;

    lv_obj_add_style(content, ui_style_card(), 0);
    lv_obj_add_style(content, ui_style_card_highlight(), 0);
    lv_obj_set_size(content, 640, 296);
    lv_obj_center(content);
    lv_obj_set_style_bg_color(content, ui_color_surface_alt(), 0);
    lv_obj_set_style_bg_grad_color(content, ui_color_surface_highlight(), 0);

    eyebrow = lv_label_create(content);
    lv_obj_add_style(eyebrow, ui_style_overline(), 0);
    lv_obj_set_width(eyebrow, 600);
    lv_obj_set_style_text_align(eyebrow, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(eyebrow, "LE BAR FESTIF");
    lv_obj_align(eyebrow, LV_ALIGN_TOP_MID, 0, 4);

    brand = lv_label_create(content);
    lv_obj_add_style(brand, ui_style_title(), 0);
    lv_obj_set_style_text_font(brand, ui_font_wordmark(), 0);
    lv_obj_set_width(brand, 600);
    lv_obj_set_style_text_align(brand, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(brand, "NECTAR");
    lv_obj_align(brand, LV_ALIGN_TOP_MID, 0, 42);

    subtitle = lv_label_create(content);
    lv_obj_add_style(subtitle, ui_style_body(), 0);
    lv_obj_set_width(subtitle, 520);
    lv_obj_set_style_text_align(subtitle, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(subtitle, LV_LABEL_LONG_WRAP);
    lv_label_set_text(subtitle, "Préparation du service et de l'expérience tactile…");
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 98);

    loading = lv_label_create(content);
    lv_obj_add_style(loading, ui_style_title(), 0);
    lv_obj_set_style_text_font(loading, ui_font_hero(), 0);
    lv_obj_set_style_text_color(loading, ui_color_accent_secondary(), 0);
    lv_label_set_text(loading, "74%");
    lv_obj_align(loading, LV_ALIGN_BOTTOM_MID, 0, -34);
    s_loading_value = loading;

    hint = lv_label_create(content);
    lv_obj_add_style(hint, ui_style_caption(), 0);
    lv_obj_set_width(hint, 520);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(hint, "On allume le bar, c'est presque prêt.");
    lv_obj_align_to(hint, loading, LV_ALIGN_OUT_TOP_MID, 0, -12);
}

void screen_boot_refresh(void)
{
    const ui_state_model_t *state = ui_state_model_get();
    uint32_t progress = (state->state_elapsed_ms * 100U) / 1400U;

    if (s_loading_value == NULL) {
        return;
    }

    if (progress > 100U) {
        progress = 100U;
    }

    lv_label_set_text_fmt(s_loading_value, "%u%%", (unsigned int)progress);
}