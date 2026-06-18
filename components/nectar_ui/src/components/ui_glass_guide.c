#include "ui_glass_guide.h"

#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"

lv_obj_t *ui_glass_guide_create(lv_obj_t *parent)
{
    lv_obj_t *guide = lv_obj_create(parent);
    lv_obj_t *glow;
    lv_obj_t *ring;
    lv_obj_t *eyebrow;
    lv_obj_t *state_label;
    lv_obj_t *hint_label;
    lv_obj_t *status_badge;

    lv_obj_add_style(guide, ui_style_card(), 0);
    lv_obj_set_size(guide, UI_CONTENT_WIDTH, 232);
    lv_obj_set_style_bg_color(guide, ui_color_surface_alt(), 0);
    lv_obj_set_style_bg_grad_color(guide, ui_color_surface_highlight(), 0);

    glow = lv_obj_create(guide);
    lv_obj_remove_style_all(glow);
    lv_obj_set_size(glow, 162, 162);
    lv_obj_align(glow, LV_ALIGN_LEFT_MID, 42, 0);
    lv_obj_set_style_radius(glow, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(glow, ui_color_accent(), 0);
    lv_obj_set_style_bg_opa(glow, LV_OPA_20, 0);
    lv_obj_set_style_shadow_width(glow, 12, 0);
    lv_obj_set_style_shadow_color(glow, ui_color_accent_glow(), 0);
    lv_obj_set_style_shadow_opa(glow, LV_OPA_20, 0);
    lv_obj_set_style_shadow_ofs_y(glow, 0, 0);

    ring = lv_obj_create(guide);
    lv_obj_remove_style_all(ring);
    lv_obj_set_size(ring, 188, 188);
    lv_obj_align(ring, LV_ALIGN_LEFT_MID, 28, 0);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ring, 8, 0);
    lv_obj_set_style_border_color(ring, ui_color_line(), 0);
    lv_obj_set_style_outline_width(ring, 1, 0);
    lv_obj_set_style_outline_pad(ring, 8, 0);
    lv_obj_set_style_outline_color(ring, ui_color_line_soft(), 0);
    lv_obj_set_style_outline_opa(ring, LV_OPA_80, 0);

    eyebrow = lv_label_create(guide);
    lv_obj_add_style(eyebrow, ui_style_overline(), 0);
    lv_label_set_text(eyebrow, "EMPLACEMENT");
    lv_obj_align(eyebrow, LV_ALIGN_TOP_LEFT, 266, 10);

    state_label = lv_label_create(guide);
    lv_obj_add_style(state_label, ui_style_title(), 0);
    lv_obj_set_width(state_label, 360);
    lv_label_set_long_mode(state_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(state_label, "Pose ton verre dans l'anneau.");
    lv_obj_align(state_label, LV_ALIGN_TOP_LEFT, 266, 42);

    hint_label = lv_label_create(guide);
    lv_obj_add_style(hint_label, ui_style_body(), 0);
    lv_label_set_text(hint_label, "Le service démarre automatiquement dès que ton verre est en place.");
    lv_label_set_long_mode(hint_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(hint_label, 402);
    lv_obj_align(hint_label, LV_ALIGN_TOP_LEFT, 266, 110);

    status_badge = lv_label_create(guide);
    lv_obj_add_style(status_badge, ui_style_badge(), 0);
    lv_label_set_text(status_badge, "EN ATTENTE");
    lv_obj_align(status_badge, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    return guide;
}

void ui_glass_guide_update(lv_obj_t *guide, bool detected)
{
    lv_obj_t *glow;
    lv_obj_t *ring;
    lv_obj_t *state_label;
    lv_obj_t *hint_label;
    lv_obj_t *status_badge;

    if (guide == NULL) {
        return;
    }

    glow = lv_obj_get_child(guide, 0);
    ring = lv_obj_get_child(guide, 1);
    state_label = lv_obj_get_child(guide, 3);
    hint_label = lv_obj_get_child(guide, 4);
    status_badge = lv_obj_get_child(guide, 5);

    if (detected) {
        lv_obj_set_style_border_color(ring, ui_color_success(), 0);
        lv_obj_set_style_outline_color(ring, ui_color_success(), 0);
        lv_obj_set_style_bg_color(glow, ui_color_success(), 0);
        lv_obj_set_style_bg_opa(glow, LV_OPA_20, 0);
        lv_label_set_text(state_label, "Verre détecté, c'est parti !");
        lv_label_set_text(hint_label, "Garde ton verre en place pendant la préparation.");
        lv_obj_add_style(status_badge, ui_style_badge(), 0);
        lv_obj_set_style_border_color(status_badge, ui_color_success(), 0);
        lv_label_set_text(status_badge, "PRÊT");
    } else {
        lv_obj_set_style_border_color(ring, ui_color_line(), 0);
        lv_obj_set_style_outline_color(ring, ui_color_line_soft(), 0);
        lv_obj_set_style_bg_color(glow, ui_color_accent(), 0);
        lv_obj_set_style_bg_opa(glow, LV_OPA_20, 0);
        lv_label_set_text(state_label, "Pose ton verre dans l'anneau.");
        lv_label_set_text(hint_label, "Le service démarre automatiquement dès que ton verre est en place.");
        lv_obj_set_style_border_color(status_badge, ui_color_line_soft(), 0);
        lv_label_set_text(status_badge, "EN ATTENTE");
    }
}