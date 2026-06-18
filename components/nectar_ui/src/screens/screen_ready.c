#include "screen_ready.h"

#include "app_controller.h"
#include "recipe_model.h"
#include "ui_chrome.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_state_model.h"

static void ready_home_cb(lv_event_t *event)
{
    (void)event;
    app_controller_acknowledge_ready();
}

void screen_ready_create(lv_obj_t *screen)
{
    const ui_state_model_t *state = ui_state_model_get();
    const recipe_model_t *recipe = app_controller_selected_recipe();
    lv_obj_t *card;
    lv_obj_t *label;
    lv_obj_t *halo;
    lv_obj_t *actions;

    ui_create_screen_header(
        screen,
        "C'EST PRÊT",
        "Ta boisson est prête !",
        "Récupère ton verre, bonne dégustation.",
        true
    );

    card = lv_obj_create(screen);
    lv_obj_add_style(card, ui_style_card(), 0);
    lv_obj_add_style(card, ui_style_card_highlight(), 0);
    lv_obj_set_size(card, 620, 214);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 136);
    lv_obj_set_style_border_color(card, ui_color_success(), 0);

    halo = lv_obj_create(card);
    lv_obj_remove_style_all(halo);
    lv_obj_set_size(halo, 102, 102);
    lv_obj_align(halo, LV_ALIGN_LEFT_MID, 54, 0);
    lv_obj_set_style_radius(halo, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(halo, ui_color_success(), 0);
    lv_obj_set_style_bg_opa(halo, LV_OPA_20, 0);
    lv_obj_set_style_shadow_width(halo, 14, 0);
    lv_obj_set_style_shadow_color(halo, ui_color_success(), 0);
    lv_obj_set_style_shadow_opa(halo, LV_OPA_20, 0);
    lv_obj_set_style_shadow_ofs_y(halo, 0, 0);

    label = lv_label_create(card);
    lv_obj_add_style(label, ui_style_overline(), 0);
    lv_label_set_text(label, "À TOI DE JOUER");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 198, 18);

    label = lv_label_create(card);
    lv_obj_add_style(label, ui_style_title(), 0);
    lv_obj_set_width(label, 330);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label, "Ta boisson t'attend.");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 198, 42);

    label = lv_label_create(card);
    lv_obj_add_style(label, ui_style_heading(), 0);
    lv_obj_set_style_text_color(label, ui_color_success(), 0);
    lv_obj_set_width(label, 330);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    if (state->selection_kind == APP_SELECTION_RECIPE && recipe != NULL) {
        lv_label_set_text_fmt(label, "%s, c'est prêt !", recipe->name);
    } else {
        lv_label_set_text(label, "Ton mélange est prêt !");
    }
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 198, 108);

    label = lv_label_create(card);
    lv_obj_add_style(label, ui_style_body(), 0);
    lv_obj_set_width(label, 330);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label, "Prends ton verre. Le menu revient tout seul dans un instant.");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 198, 142);

    actions = ui_create_action_bar(screen);
    lv_obj_align(ui_create_button(
        actions,
        "Accueil",
        UI_PRIMARY_BUTTON_WIDTH,
        UI_PRIMARY_BUTTON_HEIGHT,
        ui_style_button_primary(),
        ui_style_button_primary_pressed(),
        ready_home_cb,
        NULL
    ), LV_ALIGN_CENTER, 0, 0);
}

void screen_ready_refresh(void)
{
}