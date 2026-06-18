#include "screen_maintenance.h"

#include "app_controller.h"
#include "app_services.h"
#include "drink_model.h"
#include "ui_chrome.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_state_model.h"

static lv_obj_t *s_admin_machine_value;
static lv_obj_t *s_admin_prepare_value;
static lv_obj_t *s_admin_security_value;
static lv_obj_t *s_admin_reservoir_state[3];
static lv_obj_t *s_admin_reservoir_label[3];
static lv_obj_t *s_admin_reservoir_percent[3];

static const char *admin_machine_state_text(app_machine_state_t machine_state)
{
    switch (machine_state) {
    case APP_MACHINE_WAITING_FOR_GLASS:
        return "En attente du verre";
    case APP_MACHINE_GLASS_DETECTED:
        return "Verre détecté";
    case APP_MACHINE_PREPARING:
        return "Préparation";
    case APP_MACHINE_READY:
        return "Prêt";
    case APP_MACHINE_UNAVAILABLE:
        return "Indisponible";
    case APP_MACHINE_ERROR:
        return "Erreur";
    case APP_MACHINE_MAINTENANCE:
        return "Mode service";
    case APP_MACHINE_IDLE:
    default:
        return "Au repos";
    }
}

static void maintenance_back_cb(lv_event_t *event)
{
    (void)event;
    app_controller_leave_maintenance();
}

static lv_obj_t *maintenance_create_status_tile(lv_obj_t *parent,
                                                const char *label,
                                                lv_obj_t **value_label)
{
    lv_obj_t *tile = lv_obj_create(parent);
    lv_obj_t *eyebrow = lv_label_create(tile);
    lv_obj_t *value = lv_label_create(tile);

    lv_obj_add_style(tile, ui_style_card_inset(), 0);
    lv_obj_set_size(tile, 236, 124);

    lv_obj_add_style(eyebrow, ui_style_overline(), 0);
    lv_label_set_text(eyebrow, label);
    lv_obj_align(eyebrow, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_add_style(value, ui_style_heading(), 0);
    lv_obj_set_width(value, 200);
    lv_label_set_long_mode(value, LV_LABEL_LONG_WRAP);
    lv_label_set_text(value, "--");
    lv_obj_align(value, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    *value_label = value;
    return tile;
}

void screen_maintenance_create(lv_obj_t *screen)
{
    lv_obj_t *button;
    lv_obj_t *status_row;
    lv_obj_t *grid;
    size_t index;

    ui_create_screen_header(
        screen,
        "ESPACE SERVICE",
        "Niveaux & état",
        "Niveaux des réservoirs et état de la machine.",
        false
    );

    button = ui_create_button(
        screen,
        "Fermer le service",
        UI_SECONDARY_BUTTON_WIDTH,
        UI_SECONDARY_BUTTON_HEIGHT,
        ui_style_button_primary(),
        ui_style_button_primary_pressed(),
        maintenance_back_cb,
        NULL
    );
    lv_obj_align(button, LV_ALIGN_TOP_RIGHT, -UI_MARGIN_X, UI_MARGIN_TOP + 4);

    status_row = lv_obj_create(screen);
    lv_obj_remove_style_all(status_row);
    lv_obj_set_size(status_row, UI_CONTENT_WIDTH, 124);
    lv_obj_align(status_row, LV_ALIGN_TOP_MID, 0, 118);
    lv_obj_set_flex_flow(status_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(status_row, 18, 0);
    lv_obj_clear_flag(status_row, LV_OBJ_FLAG_SCROLLABLE);

    maintenance_create_status_tile(status_row, "Machine", &s_admin_machine_value);
    maintenance_create_status_tile(status_row, "Préparation", &s_admin_prepare_value);
    maintenance_create_status_tile(status_row, "Capteur verre", &s_admin_security_value);

    grid = lv_obj_create(screen);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, UI_CONTENT_WIDTH, 174);
    lv_obj_align(grid, LV_ALIGN_TOP_MID, 0, 266);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(grid, 18, 0);
    lv_obj_set_style_pad_all(grid, 0, 0);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

    for (index = 0; index < drink_model_count(); index++) {
        const drink_model_t *drink = drink_model_at(index);
        lv_obj_t *card = lv_obj_create(grid);
        lv_obj_t *accent;
        lv_obj_t *eyebrow;
        lv_obj_t *name;
        lv_obj_t *percent;
        lv_obj_t *state;
        lv_obj_t *hint;
        const uint8_t level = app_services_reservoir_level(index);

        lv_obj_add_style(card, ui_style_card(), 0);
        lv_obj_set_size(card, 236, 174);
        lv_obj_set_style_bg_color(card, ui_color_surface(), 0);
        lv_obj_set_style_bg_grad_color(card, ui_color_surface_alt(), 0);

        accent = lv_obj_create(card);
        lv_obj_remove_style_all(accent);
        lv_obj_set_size(accent, 3, 76);
        lv_obj_align(accent, LV_ALIGN_TOP_RIGHT, -4, 28);
        lv_obj_set_style_radius(accent, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(accent, level <= 30U ? ui_color_warning() : ui_color_accent_secondary(), 0);
        lv_obj_set_style_bg_opa(accent, LV_OPA_COVER, 0);

        eyebrow = lv_label_create(card);
        lv_obj_add_style(eyebrow, ui_style_overline(), 0);
        lv_label_set_text_fmt(eyebrow, "TANK %c", (int)('A' + (char)index));
        lv_obj_align(eyebrow, LV_ALIGN_TOP_LEFT, 0, 0);

        name = lv_label_create(card);
        lv_obj_add_style(name, ui_style_heading(), 0);
        lv_obj_set_style_text_color(name, ui_color_text_primary(), 0);
        lv_obj_set_width(name, 188);
        lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
        lv_label_set_text(name, drink->name);
        lv_obj_align(name, LV_ALIGN_TOP_LEFT, 0, 24);

        percent = lv_label_create(card);
        lv_obj_add_style(percent, ui_style_title(), 0);
        lv_label_set_text_fmt(percent, "%u%%", level);
        lv_obj_align(percent, LV_ALIGN_TOP_LEFT, 0, 56);
        s_admin_reservoir_percent[index] = percent;

        state = lv_label_create(card);
        lv_obj_add_style(state, ui_style_heading(), 0);
        lv_obj_set_width(state, 188);
        lv_label_set_long_mode(state, LV_LABEL_LONG_WRAP);
        lv_label_set_text(state, level <= 30U ? "Recharge critique" : "Niveau correct");
        lv_obj_align(state, LV_ALIGN_TOP_LEFT, 0, 102);
        s_admin_reservoir_state[index] = state;

        hint = lv_label_create(card);
        lv_obj_add_style(hint, level <= 30U ? ui_style_banner_warning() : ui_style_banner(), 0);
        lv_obj_set_width(hint, 188);
        lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
        lv_label_set_text_fmt(hint, level <= 30U ? "À recharger." : "OK pour le service.");
        lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        s_admin_reservoir_label[index] = hint;
    }

    screen_maintenance_refresh();
}

void screen_maintenance_refresh(void)
{
    const ui_state_model_t *state = ui_state_model_get();
    size_t index;

    if ((s_admin_machine_value == NULL) || (s_admin_prepare_value == NULL) || (s_admin_security_value == NULL)) {
        return;
    }

    lv_label_set_text(s_admin_machine_value, admin_machine_state_text(state->machine_state));
    lv_label_set_text_fmt(s_admin_prepare_value, "%u%%\n%s", state->prepare_progress, state->prepare_step);
    lv_label_set_text(s_admin_security_value, state->glass_detected ? "Verre en place" : "En attente du verre");

    for (index = 0; index < drink_model_count() && index < 3; index++) {
        const uint8_t level = app_services_reservoir_level(index);

        if ((s_admin_reservoir_state[index] == NULL) ||
            (s_admin_reservoir_label[index] == NULL) ||
            (s_admin_reservoir_percent[index] == NULL)) {
            continue;
        }

        lv_label_set_text(
            s_admin_reservoir_state[index],
            level <= 15U ? "Recharge immédiate" :
                (level <= 30U ? "À recharger bientôt" : "Niveau correct")
        );
        lv_obj_set_style_text_color(
            s_admin_reservoir_state[index],
            level <= 15U ? ui_color_error() :
                (level <= 30U ? ui_color_warning() : ui_color_success()),
            0
        );
        lv_label_set_text_fmt(s_admin_reservoir_percent[index], "%u%%", level);
        lv_label_set_text_fmt(
            s_admin_reservoir_label[index],
            level <= 15U ? "Niveau critique (%u%%)." :
                (level <= 30U ? "À recharger (%u%%)." : "OK (%u%%)."),
            level
        );
    }
}