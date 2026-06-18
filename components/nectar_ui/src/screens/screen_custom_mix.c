#include "screen_custom_mix.h"

#include <stdint.h>

#include "app_controller.h"
#include "drink_model.h"
#include "ui_chrome.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_state_model.h"

static lv_obj_t *s_mix_total_label;
static lv_obj_t *s_mix_slider[3];
static lv_obj_t *s_mix_value_label[3];

/* Carte un peu plus haute : crée un espace clair entre le badge "x mL au total"
 * (en haut) et la première ligne de réglage (le conteneur est aligné en bas). */
#define UI_MIX_CARD_HEIGHT 260
#define UI_MIX_CONTAINER_HEIGHT 184
#define UI_MIX_ROW_HEIGHT 56
#define UI_MIX_ROW_PAD 8
#define UI_MIX_ROW_WIDTH (UI_CONTENT_WIDTH - 36)
#define UI_MIX_NAME_WIDTH 200
#define UI_MIX_VALUE_WIDTH 84

static void custom_mix_sync_live_labels(size_t ingredient_index)
{
    const ui_state_model_t *state = ui_state_model_get();

    if (s_mix_total_label != NULL) {
        lv_label_set_text_fmt(s_mix_total_label, "%u mL au total", state->custom_total_ml);
    }

    if ((ingredient_index < 3U) && (s_mix_value_label[ingredient_index] != NULL)) {
        lv_label_set_text_fmt(s_mix_value_label[ingredient_index], "%u mL", state->custom_mix_ml[ingredient_index]);
    }
}

static void custom_mix_back_cb(lv_event_t *event)
{
    (void)event;
    app_controller_back();
}

static void custom_mix_continue_cb(lv_event_t *event)
{
    (void)event;
    app_controller_confirm_selection();
}

static void custom_mix_slider_cb(lv_event_t *event)
{
    const uintptr_t ingredient_index = (uintptr_t)lv_event_get_user_data(event);
    const int32_t value = lv_slider_get_value(lv_event_get_target(event));

    app_controller_update_custom_mix((size_t)ingredient_index, (uint16_t)value);
    custom_mix_sync_live_labels((size_t)ingredient_index);
}

void screen_custom_mix_create(lv_obj_t *screen)
{
    const ui_state_model_t *state = ui_state_model_get();
    lv_obj_t *card = lv_obj_create(screen);
    lv_obj_t *container;
    lv_obj_t *title;
    lv_obj_t *actions;
    size_t index;

    ui_create_screen_header(
        screen,
        "MON MÉLANGE",
        "Compose ton verre",
        "Règle chaque ingrédient, puis lance le service.",
        false
    );

    lv_obj_add_style(card, ui_style_card(), 0);
    lv_obj_set_size(card, UI_CONTENT_WIDTH, UI_MIX_CARD_HEIGHT);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 118);

    title = lv_label_create(card);
    lv_obj_add_style(title, ui_style_overline(), 0);
    lv_label_set_text(title, "INGRÉDIENTS");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    s_mix_total_label = lv_label_create(card);
    lv_obj_add_style(s_mix_total_label, ui_style_badge_alert(), 0);
    lv_label_set_text(s_mix_total_label, "0 mL au total");
    lv_obj_align(s_mix_total_label, LV_ALIGN_TOP_RIGHT, 0, 0);

    container = lv_obj_create(card);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, UI_CONTENT_WIDTH - 36, UI_MIX_CONTAINER_HEIGHT);
    lv_obj_align(container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(container, 6, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    for (index = 0; index < drink_model_count() && index < 3; index++) {
        const drink_model_t *drink = drink_model_at(index);
        lv_obj_t *row = lv_obj_create(container);
        lv_obj_t *name_label;
        lv_obj_t *slider;
        lv_obj_t *value_label;

        lv_obj_add_style(row, ui_style_card_inset(), 0);
        lv_obj_set_size(row, UI_MIX_ROW_WIDTH, UI_MIX_ROW_HEIGHT);
        lv_obj_set_style_pad_all(row, 10, 0);
        lv_obj_set_style_pad_column(row, 12, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        name_label = lv_label_create(row);
        slider = lv_slider_create(row);
        value_label = lv_label_create(row);

        /* Flex cell 1: ingredient name, fixed width */
        lv_obj_add_style(name_label, ui_style_heading(), 0);
        lv_obj_set_style_text_color(name_label, ui_color_text_primary(), 0);
        lv_obj_set_width(name_label, UI_MIX_NAME_WIDTH);
        lv_label_set_long_mode(name_label, LV_LABEL_LONG_DOT);
        lv_label_set_text(name_label, drink->name);

        /* Flex cell 3: mL value, fixed width, right-aligned text */
        lv_obj_add_style(value_label, ui_style_heading(), 0);
        lv_obj_set_width(value_label, UI_MIX_VALUE_WIDTH);
        lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_RIGHT, 0);
        lv_label_set_text_fmt(value_label, "%u mL", state->custom_mix_ml[index]);
        s_mix_value_label[index] = value_label;

        /* Flex cell 2: slider, grows to fill remaining width */
        lv_obj_set_flex_grow(slider, 1);
        lv_obj_set_height(slider, 10);
        lv_slider_set_range(slider, drink->min_ml, drink->max_ml);
        lv_slider_set_value(slider, state->custom_mix_ml[index], LV_ANIM_OFF);
        lv_obj_add_event_cb(slider, custom_mix_slider_cb, LV_EVENT_VALUE_CHANGED, (void *)(uintptr_t)index);
        /* Rail (piste, partie vide) bien visible avec un léger dégradé : on voit
         * clairement toute l'étendue, de 0 (à gauche) au max (à droite). */
        lv_obj_set_style_bg_color(slider, lv_color_hex(0xE7DBCC), 0);
        lv_obj_set_style_bg_grad_color(slider, lv_color_hex(0xF4EBDF), 0);
        lv_obj_set_style_bg_grad_dir(slider, LV_GRAD_DIR_HOR, 0);
        lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(slider, 1, 0);
        lv_obj_set_style_border_color(slider, lv_color_hex(0xDDCFBC), 0);
        lv_obj_set_style_bg_color(slider, ui_color_accent(), LV_PART_INDICATOR);
        lv_obj_set_style_bg_grad_color(slider, ui_color_accent_secondary(), LV_PART_INDICATOR);
        lv_obj_set_style_bg_grad_dir(slider, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(slider, ui_color_text_primary(), LV_PART_KNOB);
        lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
        lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);
        lv_obj_set_style_pad_all(slider, 6, LV_PART_KNOB);
        lv_obj_set_style_shadow_width(slider, 0, LV_PART_KNOB);
        lv_obj_set_style_shadow_color(slider, ui_color_accent_glow(), LV_PART_KNOB);
        lv_obj_set_style_shadow_opa(slider, LV_OPA_0, LV_PART_KNOB);
        s_mix_slider[index] = slider;
    }

    actions = ui_create_action_bar(screen);
    lv_obj_align(ui_create_button(
        actions,
        "Retour",
        UI_SECONDARY_BUTTON_WIDTH,
        UI_SECONDARY_BUTTON_HEIGHT,
        ui_style_button_secondary(),
        ui_style_button_secondary_pressed(),
        custom_mix_back_cb,
        NULL
    ), LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_align(ui_create_button(
        actions,
        "Servir",
        UI_PRIMARY_BUTTON_WIDTH,
        UI_PRIMARY_BUTTON_HEIGHT,
        ui_style_button_primary(),
        ui_style_button_primary_pressed(),
        custom_mix_continue_cb,
        NULL
    ), LV_ALIGN_RIGHT_MID, 0, 0);

    screen_custom_mix_refresh();
}

void screen_custom_mix_refresh(void)
{
    const ui_state_model_t *state = ui_state_model_get();
    size_t index;

    if (s_mix_total_label == NULL) {
        return;
    }

    lv_label_set_text_fmt(s_mix_total_label, "%u mL au total", state->custom_total_ml);
    for (index = 0; index < 3; index++) {
        if ((s_mix_slider[index] != NULL) && (s_mix_value_label[index] != NULL)) {
            if (lv_slider_get_value(s_mix_slider[index]) != state->custom_mix_ml[index]) {
                lv_slider_set_value(s_mix_slider[index], state->custom_mix_ml[index], LV_ANIM_OFF);
            }
            lv_label_set_text_fmt(s_mix_value_label[index], "%u mL", state->custom_mix_ml[index]);
        }
    }
}