#include "screen_home.h"

#include "app_controller.h"
#include "app_services.h"
#include "recipe_model.h"
#include "ui_chrome.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_state_model.h"

static lv_obj_t *s_home_status_banner;
static lv_obj_t *s_home_mode_value;
static lv_obj_t *s_home_recipe_value;
static lv_obj_t *s_home_reservoir_value;

static const char *home_machine_state_text(app_machine_state_t machine_state)
{
    switch (machine_state) {
    case APP_MACHINE_WAITING_FOR_GLASS:
        return "Awaiting glass";
    case APP_MACHINE_GLASS_DETECTED:
        return "Locked for service";
    case APP_MACHINE_PREPARING:
        return "Crafting now";
    case APP_MACHINE_READY:
        return "Ready for pickup";
    case APP_MACHINE_UNAVAILABLE:
        return "Temporarily paused";
    case APP_MACHINE_ERROR:
        return "Assistance required";
    case APP_MACHINE_MAINTENANCE:
        return "Service access";
    case APP_MACHINE_IDLE:
    default:
        return "Open now";
    }
}

static const char *home_banner_text(app_machine_state_t machine_state)
{
    switch (machine_state) {
    case APP_MACHINE_WAITING_FOR_GLASS:
        return "Waiting for glass";
    case APP_MACHINE_GLASS_DETECTED:
        return "Glass detected";
    case APP_MACHINE_PREPARING:
        return "Pour in progress";
    case APP_MACHINE_READY:
        return "Ready for pickup";
    case APP_MACHINE_UNAVAILABLE:
        return "One recipe paused";
    case APP_MACHINE_ERROR:
        return "Assistance required";
    case APP_MACHINE_MAINTENANCE:
        return "Admin session open";
    case APP_MACHINE_IDLE:
    default:
        return "Menu ready";
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

static lv_obj_t *create_metric_card(lv_obj_t *parent,
                                    const char *eyebrow_text,
                                    const char *label,
                                    lv_obj_t **value_label)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_t *eyebrow = lv_label_create(card);
    lv_obj_t *value = lv_label_create(card);

    (void)label;

    lv_obj_add_style(card, ui_style_card_inset(), 0);
    lv_obj_set_size(card, UI_METRIC_CARD_WIDTH, UI_METRIC_CARD_HEIGHT);

    lv_obj_add_style(eyebrow, ui_style_overline(), 0);
    lv_label_set_text(eyebrow, eyebrow_text);
    lv_obj_align(eyebrow, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_add_style(value, ui_style_heading(), 0);
    lv_obj_set_width(value, UI_METRIC_CARD_WIDTH - 28);
    lv_label_set_long_mode(value, LV_LABEL_LONG_WRAP);
    lv_label_set_text(value, "--");
    lv_obj_align(value, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    *value_label = value;
    return card;
}

void screen_home_create(lv_obj_t *screen)
{
    lv_obj_t *hero = lv_obj_create(screen);
    lv_obj_t *hero_strip;
    lv_obj_t *eyebrow;
    lv_obj_t *title;
    lv_obj_t *subtitle;
    lv_obj_t *status_row;
    lv_obj_t *actions;
    lv_obj_t *halo;

    lv_obj_add_style(hero, ui_style_card(), 0);
    lv_obj_add_style(hero, ui_style_card_highlight(), 0);
    lv_obj_set_size(hero, UI_CONTENT_WIDTH, 176);
    lv_obj_align(hero, LV_ALIGN_TOP_MID, 0, UI_MARGIN_TOP);
    lv_obj_set_style_bg_color(hero, ui_color_surface(), 0);
    lv_obj_set_style_bg_grad_color(hero, ui_color_surface_alt(), 0);
    lv_obj_set_style_bg_grad_dir(hero, LV_GRAD_DIR_VER, 0);

    hero_strip = lv_obj_create(hero);
    lv_obj_remove_style_all(hero_strip);
    lv_obj_set_size(hero_strip, 3, 140);
    lv_obj_align(hero_strip, LV_ALIGN_RIGHT_MID, -12, 0);
    lv_obj_set_style_radius(hero_strip, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(hero_strip, ui_color_accent_secondary(), 0);
    lv_obj_set_style_bg_opa(hero_strip, LV_OPA_COVER, 0);

    halo = lv_obj_create(hero);
    lv_obj_remove_style_all(halo);
    lv_obj_set_size(halo, 150, 130);
    lv_obj_align(halo, LV_ALIGN_RIGHT_MID, -34, 10);
    lv_obj_set_style_radius(halo, 22, 0);
    lv_obj_set_style_bg_color(halo, ui_color_surface_overlay(), 0);
    lv_obj_set_style_bg_grad_color(halo, ui_color_surface_highlight(), 0);
    lv_obj_set_style_bg_grad_dir(halo, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(halo, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(halo, 1, 0);
    lv_obj_set_style_border_color(halo, ui_color_line(), 0);
    lv_obj_set_style_shadow_width(halo, 8, 0);
    lv_obj_set_style_shadow_color(halo, ui_color_accent_glow(), 0);
    lv_obj_set_style_shadow_opa(halo, LV_OPA_10, 0);
    lv_obj_set_style_shadow_ofs_y(halo, 0, 0);

    eyebrow = lv_label_create(hero);
    lv_obj_add_style(eyebrow, ui_style_overline(), 0);
    lv_label_set_text(eyebrow, "EVENING CELLAR");
    lv_obj_align(eyebrow, LV_ALIGN_TOP_LEFT, 0, 0);

    title = lv_label_create(hero);
    lv_obj_add_style(title, ui_style_title(), 0);
    lv_obj_set_width(title, 360);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_label_set_text(title, "Select a signature, then serve in one smooth flow.");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 26);

    subtitle = lv_label_create(hero);
    lv_obj_add_style(subtitle, ui_style_body(), 0);
    lv_obj_set_width(subtitle, 378);
    lv_label_set_text(subtitle, "From curated cocktails to custom mixes, every pour stays guided by glass detection and clear service states.");
    lv_label_set_long_mode(subtitle, LV_LABEL_LONG_WRAP);
    lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    s_home_status_banner = lv_label_create(hero);
    lv_obj_add_style(s_home_status_banner, ui_style_banner(), 0);
    lv_obj_set_width(s_home_status_banner, LV_SIZE_CONTENT);
    lv_obj_set_style_border_color(s_home_status_banner, ui_color_accent_soft(), 0);
    lv_obj_set_style_text_align(s_home_status_banner, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(s_home_status_banner, halo, LV_ALIGN_CENTER, 0, 0);

    status_row = lv_obj_create(screen);
    lv_obj_remove_style_all(status_row);
    lv_obj_set_size(status_row, UI_CONTENT_WIDTH, UI_METRIC_CARD_HEIGHT);
    lv_obj_align(status_row, LV_ALIGN_TOP_MID, 0, 216);
    lv_obj_set_flex_flow(status_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(status_row, 16, 0);
    lv_obj_clear_flag(status_row, LV_OBJ_FLAG_SCROLLABLE);

    create_metric_card(status_row, "SERVICE", "Service status", &s_home_mode_value);
    create_metric_card(status_row, "MENU", "Signatures online", &s_home_recipe_value);
    create_metric_card(status_row, "SAFETY", "Sensor gate", &s_home_reservoir_value);

    actions = ui_create_action_bar(screen);
    lv_obj_align(ui_create_button(
        actions,
        "Custom blend",
        UI_SECONDARY_BUTTON_WIDTH,
        UI_SECONDARY_BUTTON_HEIGHT,
        ui_style_button_secondary(),
        ui_style_button_secondary_pressed(),
        home_open_mix_cb,
        NULL
    ), LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_align(ui_create_button(
        actions,
        "Open menu",
        UI_PRIMARY_BUTTON_WIDTH,
        UI_PRIMARY_BUTTON_HEIGHT,
        ui_style_button_primary(),
        ui_style_button_primary_pressed(),
        home_open_recipes_cb,
        NULL
    ), LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(ui_create_button(
        actions,
        "Service",
        UI_TERTIARY_BUTTON_WIDTH,
        UI_SECONDARY_BUTTON_HEIGHT,
        ui_style_button_secondary(),
        ui_style_button_secondary_pressed(),
        home_open_admin_cb,
        NULL
    ), LV_ALIGN_RIGHT_MID, 0, 0);

    screen_home_refresh();
}

void screen_home_refresh(void)
{
    const ui_state_model_t *state = ui_state_model_get();
    size_t available_recipes = 0;
    size_t index;

    if ((s_home_status_banner == NULL) || (s_home_mode_value == NULL) ||
        (s_home_recipe_value == NULL) || (s_home_reservoir_value == NULL)) {
        return;
    }

    for (index = 0; index < recipe_model_count(); index++) {
        const recipe_model_t *recipe = recipe_model_at(index);

        if (recipe != NULL && recipe->available) {
            available_recipes++;
        }
    }

    lv_label_set_text_fmt(s_home_mode_value, "%s", home_machine_state_text(state->machine_state));
    lv_label_set_text_fmt(s_home_recipe_value, "%u signatures live", (unsigned int)available_recipes);
    lv_label_set_text_fmt(
        s_home_reservoir_value,
        state->glass_detected ? "Lock confirmed" : "Waiting for glass"
    );
    lv_label_set_text(s_home_status_banner, home_banner_text(state->machine_state));
    lv_obj_set_style_border_color(
        s_home_status_banner,
        state->machine_state == APP_MACHINE_ERROR ? ui_color_error() :
            (state->machine_state == APP_MACHINE_UNAVAILABLE ? ui_color_warning() : ui_color_success()),
        0
    );
}