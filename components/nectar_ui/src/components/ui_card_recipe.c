#include "ui_card_recipe.h"

#include <ctype.h>
#include <string.h>

#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_typography.h"

static void recipe_card_monogram(const char *name, char *buffer, size_t buffer_size)
{
    size_t length = 0;
    bool take_next = true;

    if ((buffer == NULL) || (buffer_size < 2U)) {
        return;
    }

    for (; (name != NULL) && (*name != '\0') && (length < (buffer_size - 1U)); name++) {
        if (isalpha((unsigned char)*name) && take_next) {
            buffer[length++] = (char)toupper((unsigned char)*name);
            take_next = false;
        } else if ((*name == ' ') || (*name == '-') || (*name == '/')) {
            take_next = true;
        }

        if (length >= 2U) {
            break;
        }
    }

    if (length == 0U) {
        buffer[length++] = 'N';
    }

    buffer[length] = '\0';
}

static const char *recipe_card_status_text(const recipe_model_t *recipe)
{
    if (!recipe->available) {
        return "Indisponible";
    }

    if (recipe->stock_percent <= 35U) {
        return "Bientôt épuisé";
    }

    return "Disponible";
}

lv_obj_t *ui_card_recipe_create(lv_obj_t *parent,
                                const recipe_model_t *recipe,
                                size_t family_index,
                                lv_event_cb_t event_cb,
                                void *user_data)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_t *status;
    lv_obj_t *media_shell;
    lv_obj_t *media_frame;
    lv_obj_t *media_art;
    lv_obj_t *halo;
    lv_obj_t *monogram;
    lv_obj_t *title;
    lv_obj_t *subtitle;
    lv_obj_t *footer;
    char monogram_text[3] = {0};

    recipe_card_monogram(recipe->name, monogram_text, sizeof(monogram_text));

    lv_obj_remove_style_all(button);
    lv_obj_set_size(button, UI_RECIPE_CARD_WIDTH, UI_RECIPE_CARD_HEIGHT);
    lv_obj_add_style(button, ui_style_card(), 0);
    lv_obj_add_event_cb(button, event_cb, LV_EVENT_CLICKED, user_data);
    lv_obj_clear_flag(button, LV_OBJ_FLAG_SCROLLABLE);

    if (!recipe->available) {
        lv_obj_set_style_bg_color(button, ui_color_surface_overlay(), 0);
        lv_obj_set_style_bg_grad_color(button, ui_color_surface_alt(), 0);
        lv_obj_set_style_border_color(button, ui_color_error(), 0);
        lv_obj_set_style_outline_color(button, ui_color_error(), 0);
        lv_obj_set_style_shadow_color(button, ui_color_surface_overlay(), 0);
    } else if (recipe->stock_percent <= 35U) {
        lv_obj_set_style_border_color(button, ui_color_warning(), 0);
        lv_obj_set_style_outline_color(button, ui_color_warning(), 0);
    }

    /* Statut seul, centré en haut (le libellé "SÉLECTION" a été retiré pour ne pas
     * chevaucher le statut sur une carte étroite). */
    status = lv_label_create(button);
    lv_obj_add_style(status, recipe->available ?
        (recipe->stock_percent <= 35U ? ui_style_badge_alert() : ui_style_badge()) :
        ui_style_badge_alert(), 0);
    lv_label_set_text(status, recipe_card_status_text(recipe));
    if (!recipe->available) {
        lv_obj_set_style_border_color(status, ui_color_error(), 0);
    }
    lv_obj_align(status, LV_ALIGN_TOP_MID, 0, 0);

    media_shell = lv_obj_create(button);
    lv_obj_remove_style_all(media_shell);
    lv_obj_add_style(media_shell, ui_style_card_inset(), 0);
    lv_obj_set_size(media_shell, 148, 86);
    lv_obj_align(media_shell, LV_ALIGN_TOP_MID, 0, 34);
    lv_obj_clear_flag(media_shell, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(media_shell, 0, 0);

    media_frame = lv_obj_create(media_shell);
    lv_obj_remove_style_all(media_frame);
    lv_obj_set_size(media_frame, 132, 72);
    lv_obj_center(media_frame);
    lv_obj_set_style_radius(media_frame, 16, 0);
    lv_obj_set_style_bg_color(media_frame, ui_color_surface_overlay(), 0);
    lv_obj_set_style_bg_opa(media_frame, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(media_frame, 1, 0);
    lv_obj_set_style_border_color(media_frame, ui_color_line_soft(), 0);
    lv_obj_clear_flag(media_frame, LV_OBJ_FLAG_SCROLLABLE);

    media_art = NULL;
    if (recipe->visual_src != NULL) {
        media_art = lv_img_create(media_frame);
        lv_img_set_src(media_art, recipe->visual_src);
        lv_obj_center(media_art);

        halo = lv_obj_create(media_frame);
        lv_obj_remove_style_all(halo);
        lv_obj_set_size(halo, 120, 18);
        lv_obj_align(halo, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_color(halo, ui_color_surface_overlay(), 0);
        lv_obj_set_style_bg_opa(halo, LV_OPA_40, 0);
        lv_obj_set_style_radius(halo, 16, 0);
    } else {
        halo = lv_obj_create(media_frame);
        lv_obj_remove_style_all(halo);
        lv_obj_set_size(halo, 62, 62);
        lv_obj_align(halo, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_radius(halo, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(halo, recipe->available ? ui_color_family(family_index) : ui_color_disabled(), 0);
        lv_obj_set_style_bg_opa(halo, recipe->available ? LV_OPA_COVER : LV_OPA_40, 0);
        lv_obj_set_style_shadow_width(halo, 10, 0);
        lv_obj_set_style_shadow_color(halo, recipe->available ? ui_color_family(family_index) : ui_color_disabled(), 0);
        lv_obj_set_style_shadow_opa(halo, recipe->available ? LV_OPA_30 : LV_OPA_10, 0);
        lv_obj_set_style_shadow_ofs_y(halo, 0, 0);

        monogram = lv_label_create(media_frame);
        lv_obj_add_style(monogram, ui_style_title(), 0);
        lv_obj_set_style_text_font(monogram, ui_font_display(), 0);
        lv_obj_set_style_text_color(monogram, recipe->available ? ui_color_text_ink() : ui_color_text_muted(), 0);
        lv_label_set_text(monogram, monogram_text);
        lv_obj_center(monogram);
    }

    /*
     * Bloc texte : titre + tagline + footer enchaînés du haut vers le bas.
     * media_shell bas = content_y 120 ; titre à content_y 126 (gap 6).
     */
    title = lv_label_create(button);
    lv_obj_add_style(title, ui_style_heading(), 0);
    lv_obj_set_style_text_color(title, recipe->available ? ui_color_text_primary() : ui_color_text_secondary(), 0);
    lv_obj_set_width(title, 160);
    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(title, recipe->name);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 126);

    subtitle = lv_label_create(button);
    lv_obj_add_style(subtitle, ui_style_caption(), 0);
    lv_label_set_long_mode(subtitle, LV_LABEL_LONG_DOT);
    lv_obj_set_width(subtitle, 160);
    lv_obj_set_style_text_align(subtitle, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(subtitle, recipe->tagline);
    lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);

    footer = lv_label_create(button);
    lv_obj_add_style(footer, ui_style_caption(), 0);
    lv_obj_set_width(footer, 164);
    lv_obj_set_style_text_align(footer, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(
        footer,
        recipe->available ?
            (recipe->stock_percent <= 35U ? ui_color_warning() : ui_color_text_secondary()) :
            ui_color_error(),
        0
    );
    lv_label_set_text_fmt(
        footer,
        recipe->available ? "%u mL · %u ingrédient%s" : "Indisponible",
        recipe->total_ml,
        recipe->ingredient_count,
        recipe->ingredient_count > 1U ? "s" : ""
    );
    lv_obj_align_to(footer, subtitle, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);

    return button;
}
