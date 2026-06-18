#include "ui_progress_panel.h"

#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_typography.h"

lv_obj_t *ui_progress_panel_create(lv_obj_t *parent)
{
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_t *eyebrow;
    lv_obj_t *title;
    lv_obj_t *status;
    lv_obj_t *percent;
    lv_obj_t *summary;
    lv_obj_t *step;

    lv_obj_add_style(panel, ui_style_card(), 0);
    lv_obj_set_size(panel, UI_CONTENT_WIDTH, 194);
    lv_obj_set_style_bg_color(panel, ui_color_surface_alt(), 0);
    lv_obj_set_style_bg_grad_color(panel, ui_color_surface_highlight(), 0);

    eyebrow = lv_label_create(panel);
    lv_obj_add_style(eyebrow, ui_style_overline(), 0);
    lv_label_set_text(eyebrow, "PRÉPARATION");
    lv_obj_align(eyebrow, LV_ALIGN_TOP_LEFT, 0, 0);

    title = lv_label_create(panel);
    lv_obj_add_style(title, ui_style_title(), 0);
    lv_label_set_text(title, "On prépare ton verre…");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 28);

    status = lv_label_create(panel);
    lv_obj_add_style(status, ui_style_badge(), 0);
    lv_label_set_text(status, "VERRE EN PLACE");
    lv_obj_align(status, LV_ALIGN_TOP_RIGHT, 0, -2);

    percent = lv_label_create(panel);
    lv_obj_add_style(percent, ui_style_title(), 0);
    lv_obj_set_style_text_font(percent, ui_font_hero(), 0);
    lv_obj_set_style_text_color(percent, ui_color_accent(), 0);
    lv_label_set_text(percent, "0%");
    lv_obj_align(percent, LV_ALIGN_TOP_RIGHT, 0, 26);

    summary = lv_label_create(panel);
    lv_obj_add_style(summary, ui_style_body(), 0);
    lv_obj_set_width(summary, 520);
    lv_label_set_long_mode(summary, LV_LABEL_LONG_WRAP);
    lv_label_set_text(summary, "Ta boisson se prépare, plus que quelques secondes.");
    lv_obj_align(summary, LV_ALIGN_TOP_LEFT, 0, 90);

    step = lv_label_create(panel);
    lv_obj_add_style(step, ui_style_heading(), 0);
    lv_obj_set_width(step, 680);
    lv_label_set_long_mode(step, LV_LABEL_LONG_WRAP);
    lv_label_set_text(step, "Démarrage…");
    lv_obj_align(step, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    return panel;
}

void ui_progress_panel_update(lv_obj_t *panel, uint8_t progress, const char *step_text)
{
    lv_obj_t *percent;
    lv_obj_t *step;

    if (panel == NULL) {
        return;
    }

    percent = lv_obj_get_child(panel, 3);
    step = lv_obj_get_child(panel, 5);

    lv_label_set_text_fmt(percent, "%u%%", progress);
    lv_label_set_text(step, step_text);
}