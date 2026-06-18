#include "screen_error.h"

#include "app_controller.h"
#include "ui_chrome.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_state_model.h"

static void error_home_cb(lv_event_t *event)
{
    (void)event;
    app_controller_open_home();
}

void screen_error_create(lv_obj_t *screen)
{
    const ui_state_model_t *state = ui_state_model_get();
    lv_obj_t *card;
    lv_obj_t *title;
    lv_obj_t *message;
    lv_obj_t *actions;

    ui_create_screen_header(
        screen,
        "INFO",
        "Petit souci",
        "Pas de panique, voici quoi faire.",
        true
    );

    card = lv_obj_create(screen);
    lv_obj_add_style(card, ui_style_card(), 0);
    lv_obj_set_width(card, 620);
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_bottom(card, 20, 0);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 138);
    lv_obj_set_style_border_color(card, ui_color_error(), 0);
    lv_obj_set_style_outline_color(card, ui_color_error(), 0);

    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(card, 10, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    title = lv_label_create(card);
    lv_obj_add_style(title, ui_style_overline(), 0);
    lv_label_set_text(title, "ATTENTION");

    title = lv_label_create(card);
    lv_obj_add_style(title, ui_style_title(), 0);
    lv_obj_set_width(title, 560);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_label_set_text(title, state->error_title);

    message = lv_label_create(card);
    lv_obj_add_style(message, ui_style_body(), 0);
    lv_obj_set_width(message, 560);
    lv_label_set_long_mode(message, LV_LABEL_LONG_WRAP);
    lv_label_set_text(message, state->error_message);

    actions = ui_create_action_bar(screen);
    lv_obj_align(ui_create_button(
        actions,
        "Accueil",
        UI_PRIMARY_BUTTON_WIDTH,
        UI_PRIMARY_BUTTON_HEIGHT,
        ui_style_button_primary(),
        ui_style_button_primary_pressed(),
        error_home_cb,
        NULL
    ), LV_ALIGN_CENTER, 0, 0);
}

void screen_error_refresh(void)
{
}