#include "screen_home.h"

#include "app_controller.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_typography.h"
#include "ui_state_model.h"

static lv_obj_t *s_home_status_chip;

static const char *home_status_text(app_machine_state_t machine_state)
{
    switch (machine_state) {
    case APP_MACHINE_ERROR:        return "Indisponible";
    case APP_MACHINE_UNAVAILABLE:  return "Un cocktail en pause";
    case APP_MACHINE_MAINTENANCE:  return "Mode service";
    default:                       return "Prêt à servir";
    }
}

static void home_open_recipes_cb(lv_event_t *event)
{
    (void)event;
    app_controller_open_recipes();
}

static void home_open_mix_cb(lv_event_t *event)
{
    (void)event;
    app_controller_open_custom_mix();
}

static void home_open_admin_cb(lv_event_t *event)
{
    (void)event;
    app_controller_open_admin_auth();
}

static lv_obj_t *home_create_tile(lv_obj_t *parent,
                                  const char *title_text,
                                  const char *subtitle_text,
                                  bool primary,
                                  lv_event_cb_t event_cb)
{
    lv_obj_t *tile = lv_btn_create(parent);
    lv_obj_t *title = lv_label_create(tile);
    lv_obj_t *subtitle = lv_label_create(tile);

    lv_obj_remove_style_all(tile);
    lv_obj_add_style(tile, ui_style_card(), 0);
    lv_obj_set_size(tile, 356, 196);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(tile, event_cb, LV_EVENT_CLICKED, NULL);

    if (primary) {
        lv_obj_set_style_bg_color(tile, ui_color_accent(), 0);
        lv_obj_set_style_bg_grad_color(tile, ui_color_accent_secondary(), 0);
        lv_obj_set_style_bg_grad_dir(tile, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_border_width(tile, 0, 0);
        lv_obj_set_style_shadow_color(tile, ui_color_accent_glow(), 0);
        lv_obj_set_style_shadow_opa(tile, LV_OPA_40, 0);
    } else {
        lv_obj_set_style_border_width(tile, 2, 0);
        lv_obj_set_style_border_color(tile, lv_color_hex(0xFFD9B8), 0);
    }

    lv_obj_add_style(title, ui_style_title(), 0);
    lv_obj_set_style_text_color(title, primary ? ui_color_text_ink() : ui_color_accent(), 0);
    lv_obj_set_width(title, 312);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_label_set_text(title, title_text);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 22, 28);

    lv_obj_add_style(subtitle, ui_style_body(), 0);
    lv_obj_set_style_text_color(subtitle, primary ? ui_color_text_ink() : ui_color_text_secondary(), 0);
    lv_obj_set_width(subtitle, 312);
    lv_label_set_long_mode(subtitle, LV_LABEL_LONG_WRAP);
    lv_label_set_text(subtitle, subtitle_text);
    lv_obj_align(subtitle, LV_ALIGN_TOP_LEFT, 22, 110);

    return tile;
}

void screen_home_create(lv_obj_t *screen)
{
    lv_obj_t *eyebrow;
    lv_obj_t *greeting;
    lv_obj_t *tiles;
    lv_obj_t *service_btn;
    lv_obj_t *service_label;

    eyebrow = lv_label_create(screen);
    lv_obj_add_style(eyebrow, ui_style_overline(), 0);
    lv_label_set_text(eyebrow, "NECTAR · LE BAR");
    lv_obj_align(eyebrow, LV_ALIGN_TOP_LEFT, UI_MARGIN_X, UI_MARGIN_TOP);

    greeting = lv_label_create(screen);
    lv_obj_add_style(greeting, ui_style_title(), 0);
    lv_obj_set_style_text_font(greeting, ui_font_display(), 0);
    lv_obj_set_width(greeting, 470);
    lv_label_set_long_mode(greeting, LV_LABEL_LONG_WRAP);
    lv_label_set_text(greeting, "Qu'est-ce qui te ferait plaisir ?");
    lv_obj_align(greeting, LV_ALIGN_TOP_LEFT, UI_MARGIN_X, UI_MARGIN_TOP + 22);

    s_home_status_chip = lv_label_create(screen);
    lv_obj_add_style(s_home_status_chip, ui_style_badge(), 0);
    lv_obj_set_style_bg_color(s_home_status_chip, lv_color_hex(0xE6F6EA), 0);
    lv_obj_set_style_text_color(s_home_status_chip, ui_color_success(), 0);
    lv_obj_align(s_home_status_chip, LV_ALIGN_TOP_RIGHT, -UI_MARGIN_X, UI_MARGIN_TOP);

    tiles = lv_obj_create(screen);
    lv_obj_remove_style_all(tiles);
    lv_obj_set_size(tiles, UI_CONTENT_WIDTH, 196);
    lv_obj_align(tiles, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_set_flex_flow(tiles, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(tiles, 32, 0);
    lv_obj_clear_flag(tiles, LV_OBJ_FLAG_SCROLLABLE);

    home_create_tile(tiles, "Le Menu", "Nos cocktails prêts à servir.", true, home_open_recipes_cb);
    home_create_tile(tiles, "Composer mon verre", "Crée ton propre mélange.", false, home_open_mix_cb);

    service_btn = lv_btn_create(screen);
    lv_obj_remove_style_all(service_btn);
    lv_obj_add_style(service_btn, ui_style_button_secondary(), 0);
    lv_obj_add_style(service_btn, ui_style_button_secondary_pressed(), LV_STATE_PRESSED);
    lv_obj_set_size(service_btn, 160, 52);
    lv_obj_align(service_btn, LV_ALIGN_BOTTOM_RIGHT, -UI_MARGIN_X, -UI_MARGIN_BOTTOM);
    lv_obj_add_event_cb(service_btn, home_open_admin_cb, LV_EVENT_CLICKED, NULL);
    service_label = lv_label_create(service_btn);
    lv_label_set_text(service_label, "Service");
    lv_obj_center(service_label);

    screen_home_refresh();
}

void screen_home_refresh(void)
{
    const ui_state_model_t *state = ui_state_model_get();

    if (s_home_status_chip == NULL) {
        return;
    }

    lv_label_set_text(s_home_status_chip, home_status_text(state->machine_state));
    if (state->machine_state == APP_MACHINE_ERROR) {
        lv_obj_set_style_bg_color(s_home_status_chip, lv_color_hex(0xFCE4E4), 0);
        lv_obj_set_style_text_color(s_home_status_chip, ui_color_error(), 0);
    } else if (state->machine_state == APP_MACHINE_UNAVAILABLE) {
        lv_obj_set_style_bg_color(s_home_status_chip, lv_color_hex(0xFFF3D6), 0);
        lv_obj_set_style_text_color(s_home_status_chip, lv_color_hex(0x9A6500), 0);
    } else {
        lv_obj_set_style_bg_color(s_home_status_chip, lv_color_hex(0xE6F6EA), 0);
        lv_obj_set_style_text_color(s_home_status_chip, ui_color_success(), 0);
    }
}
