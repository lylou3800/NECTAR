#include "screen_drink_detail.h"

#include <ctype.h>

#include "app_controller.h"
#include "recipe_model.h"
#include "ui_allergen_badge.h"
#include "ui_chrome.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_typography.h"

static void detail_monogram(const char *name, char *buffer, size_t buffer_size)
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

static void detail_back_cb(lv_event_t *event)
{
    (void)event;
    app_controller_back();
}

static void detail_continue_cb(lv_event_t *event)
{
    (void)event;
    app_controller_confirm_selection();
}

void screen_drink_detail_create(lv_obj_t *screen)
{
    const recipe_model_t *recipe = app_controller_selected_recipe();
    lv_obj_t *hero;
    lv_obj_t *summary;
    lv_obj_t *container;
    lv_obj_t *title;
    lv_obj_t *subtitle;
    lv_obj_t *actions;
    lv_obj_t *halo;
    lv_obj_t *badge;
    uint8_t index;
    char monogram_text[3] = {0};

    if (recipe == NULL) {
        ui_create_screen_header(screen, "LA RECETTE", "Cocktail introuvable", "Reviens au menu et choisis-en un autre.", true);
        return;
    }

    detail_monogram(recipe->name, monogram_text, sizeof(monogram_text));

    ui_create_screen_header(screen, "LA RECETTE", recipe->name, recipe->tagline, false);

    hero = lv_obj_create(screen);
    lv_obj_add_style(hero, ui_style_card(), 0);
    lv_obj_add_style(hero, ui_style_card_highlight(), 0);
    lv_obj_set_size(hero, 276, 252);
    lv_obj_align(hero, LV_ALIGN_TOP_LEFT, UI_MARGIN_X, 118);
    lv_obj_set_style_bg_color(hero, ui_color_surface_alt(), 0);
    lv_obj_set_style_bg_grad_color(hero, ui_color_surface_highlight(), 0);

    title = lv_label_create(hero);
    lv_obj_add_style(title, ui_style_overline(), 0);
    lv_label_set_text(title, "À DÉCOUVRIR");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    badge = lv_label_create(hero);
    lv_obj_add_style(badge, recipe->stock_percent <= 35U ? ui_style_badge_alert() : ui_style_badge(), 0);
    lv_label_set_text(badge, recipe->stock_percent <= 35U ? "BIENTÔT ÉPUISÉ" : "PRÊT");
    lv_obj_align(badge, LV_ALIGN_TOP_RIGHT, 0, 0);

    halo = lv_obj_create(hero);
    lv_obj_remove_style_all(halo);
    lv_obj_set_size(halo, 112, 112);
    lv_obj_align(halo, LV_ALIGN_TOP_MID, 0, 42);
    lv_obj_set_style_radius(halo, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(halo, ui_color_accent(), 0);
    lv_obj_set_style_bg_opa(halo, LV_OPA_20, 0);
    lv_obj_set_style_shadow_width(halo, 14, 0);
    lv_obj_set_style_shadow_color(halo, ui_color_accent_glow(), 0);
    lv_obj_set_style_shadow_opa(halo, LV_OPA_20, 0);
    lv_obj_set_style_shadow_ofs_y(halo, 0, 0);

    subtitle = lv_label_create(hero);
    lv_obj_add_style(subtitle, ui_style_title(), 0);
    lv_obj_set_style_text_font(subtitle, ui_font_display(), 0);
    lv_label_set_text(subtitle, monogram_text);
    lv_obj_align_to(subtitle, halo, LV_ALIGN_CENTER, 0, 0);

    subtitle = lv_label_create(hero);
    lv_obj_add_style(subtitle, ui_style_heading(), 0);
    lv_obj_set_width(subtitle, 220);
    lv_obj_set_style_text_align(subtitle, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text_fmt(subtitle, "%u mL par verre", recipe->total_ml);
    lv_obj_align(subtitle, LV_ALIGN_BOTTOM_MID, 0, -66);

    container = lv_obj_create(hero);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, 220, 54);
    lv_obj_align(container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_gap(container, 8, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    for (index = 0; index < recipe->allergen_count; index++) {
        ui_allergen_badge_create(container, recipe->allergens[index]);
    }

    summary = lv_obj_create(screen);
    lv_obj_add_style(summary, ui_style_card(), 0);
    lv_obj_set_size(summary, 448, 252);
    lv_obj_align(summary, LV_ALIGN_TOP_RIGHT, -UI_MARGIN_X, 118);

    title = lv_label_create(summary);
    lv_obj_add_style(title, ui_style_overline(), 0);
    lv_label_set_text(title, "COMPOSITION");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    subtitle = lv_label_create(summary);
    lv_obj_add_style(subtitle, ui_style_body(), 0);
    lv_obj_set_width(subtitle, 390);
    lv_label_set_long_mode(subtitle, LV_LABEL_LONG_WRAP);
    lv_label_set_text(subtitle, "Composition d'un verre : ingrédients et allergènes en clair.");
    lv_obj_align(subtitle, LV_ALIGN_TOP_LEFT, 0, 26);

    container = lv_obj_create(summary);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, 410, 124);
    lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 90);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(container, 10, 0);

    for (index = 0; index < recipe->ingredient_count; index++) {
        lv_obj_t *row = lv_obj_create(container);
        lv_obj_t *name = lv_label_create(row);
        lv_obj_t *amount = lv_label_create(row);

        lv_obj_add_style(row, ui_style_card_inset(), 0);
        lv_obj_set_size(row, 410, 34);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_add_style(name, ui_style_body(), 0);
        lv_obj_set_style_text_color(name, ui_color_text_primary(), 0);
        lv_label_set_text(name, recipe->ingredients[index].name);
        lv_obj_align(name, LV_ALIGN_LEFT_MID, 0, 0);

        lv_obj_add_style(amount, ui_style_heading(), 0);
        lv_label_set_text_fmt(amount, "%u mL", recipe->ingredients[index].amount_ml);
        lv_obj_align(amount, LV_ALIGN_RIGHT_MID, 0, 0);
    }

    title = lv_label_create(summary);
    lv_obj_add_style(title, ui_style_heading(), 0);
    lv_label_set_text_fmt(title, "%u mL au total", recipe->total_ml);
    lv_obj_align(title, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    actions = ui_create_action_bar(screen);
    lv_obj_align(ui_create_button(
        actions,
        "Retour",
        UI_SECONDARY_BUTTON_WIDTH,
        UI_SECONDARY_BUTTON_HEIGHT,
        ui_style_button_secondary(),
        ui_style_button_secondary_pressed(),
        detail_back_cb,
        NULL
    ), LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_align(ui_create_button(
        actions,
        "Servir",
        UI_PRIMARY_BUTTON_WIDTH,
        UI_PRIMARY_BUTTON_HEIGHT,
        ui_style_button_primary(),
        ui_style_button_primary_pressed(),
        detail_continue_cb,
        NULL
    ), LV_ALIGN_RIGHT_MID, 0, 0);
}

void screen_drink_detail_refresh(void)
{
}