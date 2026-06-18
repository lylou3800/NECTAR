#include "screen_maintenance.h"

#include "app_controller.h"
#include "app_services.h"
#include "drink_model.h"
#include "ui_chrome.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_typography.h"
#include "ui_state_model.h"

static lv_obj_t *s_admin_reservoir_state[3];
static lv_obj_t *s_admin_reservoir_percent[3];
static lv_obj_t *s_svc_glass_btn;
static lv_obj_t *s_svc_serve_btn;
static lv_obj_t *s_svc_pump_status;

static void maintenance_back_cb(lv_event_t *event)
{
    (void)event;
    app_controller_leave_maintenance();
}

static void svc_glass_check_cb(lv_event_t *event)
{
    (void)event;
    app_controller_svc_check_glass();
    screen_maintenance_refresh();
}

static void svc_serve_cb(lv_event_t *event)
{
    (void)event;
    app_controller_svc_start_pump();
    screen_maintenance_refresh();
}

void screen_maintenance_create(lv_obj_t *screen)
{
    lv_obj_t *button;
    lv_obj_t *grid;
    lv_obj_t *test_eyebrow;
    size_t index;

    s_svc_glass_btn = NULL;
    s_svc_serve_btn = NULL;
    s_svc_pump_status = NULL;

    ui_create_screen_header(
        screen,
        "ESPACE SERVICE",
        "Niveaux des réservoirs",
        "Niveau des réservoirs et test du service.",
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

    /* Rangée des niveaux de réservoirs (TANK A / B / C). */
    grid = lv_obj_create(screen);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, UI_CONTENT_WIDTH, 168);
    lv_obj_align(grid, LV_ALIGN_TOP_MID, 0, 118);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(grid, 18, 0);
    lv_obj_set_style_pad_all(grid, 0, 0);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

    for (index = 0; index < drink_model_count(); index++) {
        const drink_model_t *drink = drink_model_at(index);
        lv_obj_t *card = lv_obj_create(grid);
        lv_obj_t *inner;
        lv_obj_t *eyebrow;
        lv_obj_t *name;
        lv_obj_t *percent;
        lv_obj_t *state;
        const uint8_t level = app_services_reservoir_level(index);

        lv_obj_add_style(card, ui_style_card(), 0);
        lv_obj_set_size(card, 236, 168);
        lv_obj_set_style_bg_color(card, ui_color_surface(), 0);
        lv_obj_set_style_bg_grad_color(card, ui_color_surface_alt(), 0);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

        inner = lv_obj_create(card);
        lv_obj_remove_style_all(inner);
        lv_obj_set_size(inner, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(inner, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_gap(inner, 6, 0);
        lv_obj_set_style_pad_all(inner, 0, 0);
        lv_obj_clear_flag(inner, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(inner, LV_ALIGN_TOP_LEFT, 0, 0);

        eyebrow = lv_label_create(inner);
        lv_obj_add_style(eyebrow, ui_style_overline(), 0);
        lv_obj_set_width(eyebrow, 200);
        lv_label_set_long_mode(eyebrow, LV_LABEL_LONG_DOT);
        lv_label_set_text_fmt(eyebrow, "TANK %c", (int)('A' + (char)index));

        name = lv_label_create(inner);
        lv_obj_add_style(name, ui_style_heading(), 0);
        lv_obj_set_style_text_color(name, ui_color_text_primary(), 0);
        lv_obj_set_width(name, 200);
        lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
        lv_label_set_text(name, drink->name);

        percent = lv_label_create(inner);
        lv_obj_add_style(percent, ui_style_title(), 0);
        lv_obj_set_width(percent, 200);
        lv_label_set_long_mode(percent, LV_LABEL_LONG_DOT);
        lv_label_set_text_fmt(percent, "%u%%", level);
        s_admin_reservoir_percent[index] = percent;

        state = lv_label_create(inner);
        lv_obj_add_style(state, ui_style_heading(), 0);
        lv_obj_set_width(state, 200);
        lv_label_set_long_mode(state, LV_LABEL_LONG_DOT);
        s_admin_reservoir_state[index] = state;
    }

    /* --- Panneau de test du service : simulateur pompe + présence de verre --- */
    test_eyebrow = lv_label_create(screen);
    lv_obj_add_style(test_eyebrow, ui_style_overline(), 0);
    lv_label_set_text(test_eyebrow, "TEST DU SERVICE");
    lv_obj_align(test_eyebrow, LV_ALIGN_TOP_LEFT, UI_MARGIN_X, 298);

    /* Bouton : lit le capteur de verre réel (MCP23017). */
    s_svc_glass_btn = ui_create_button(
        screen,
        "Vérifier le verre",
        332,
        60,
        ui_style_button_secondary(),
        ui_style_button_secondary_pressed(),
        svc_glass_check_cb,
        NULL
    );
    lv_obj_align(s_svc_glass_btn, LV_ALIGN_TOP_LEFT, UI_MARGIN_X, 322);

    /* Bouton de test : lance la pompe 10 s (seulement si un verre est présent). */
    s_svc_serve_btn = ui_create_button(
        screen,
        "Tester le service (10 s)",
        332,
        60,
        ui_style_button_primary(),
        ui_style_button_primary_pressed(),
        svc_serve_cb,
        NULL
    );
    lv_obj_align(s_svc_serve_btn, LV_ALIGN_TOP_RIGHT, -UI_MARGIN_X, 322);

    s_svc_pump_status = lv_label_create(screen);
    lv_obj_add_style(s_svc_pump_status, ui_style_body(), 0);
    lv_obj_set_width(s_svc_pump_status, UI_CONTENT_WIDTH);
    lv_obj_set_style_text_align(s_svc_pump_status, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_svc_pump_status, LV_ALIGN_TOP_MID, 0, 392);

    screen_maintenance_refresh();
}

void screen_maintenance_refresh(void)
{
    const ui_state_model_t *state = ui_state_model_get();
    size_t index;

    if (s_admin_reservoir_percent[0] == NULL) {
        return;
    }

    for (index = 0; index < drink_model_count() && index < 3; index++) {
        const uint8_t level = app_services_reservoir_level(index);
        const lv_color_t level_color = level <= 15U ? ui_color_error() :
            (level <= 30U ? ui_color_warning() : ui_color_success());

        if ((s_admin_reservoir_state[index] == NULL) || (s_admin_reservoir_percent[index] == NULL)) {
            continue;
        }

        lv_label_set_text_fmt(s_admin_reservoir_percent[index], "%u%%", level);
        lv_obj_set_style_text_color(s_admin_reservoir_percent[index], level_color, 0);

        lv_label_set_text(
            s_admin_reservoir_state[index],
            level <= 15U ? "Critique" : (level <= 30U ? "À recharger" : "Niveau correct")
        );
        lv_obj_set_style_text_color(s_admin_reservoir_state[index], level_color, 0);
    }

    /* Bouton "Vérifier le verre" : reflète la dernière lecture du capteur réel. */
    if (s_svc_glass_btn != NULL) {
        lv_obj_t *glass_lbl = lv_obj_get_child(s_svc_glass_btn, 0);

        if (state->svc_test_glass_present) {
            lv_label_set_text(glass_lbl, "Verre : présent");
            lv_obj_set_style_bg_color(s_svc_glass_btn, lv_color_hex(0xE6F6EA), 0);
            lv_obj_set_style_text_color(s_svc_glass_btn, ui_color_success(), 0);
            lv_obj_set_style_border_color(s_svc_glass_btn, ui_color_success(), 0);
        } else {
            lv_label_set_text(glass_lbl, "Vérifier le verre");
            lv_obj_set_style_bg_color(s_svc_glass_btn, ui_color_surface_alt(), 0);
            lv_obj_set_style_text_color(s_svc_glass_btn, ui_color_text_muted(), 0);
            lv_obj_set_style_border_color(s_svc_glass_btn, ui_color_line(), 0);
        }
    }

    /* Bouton "Tester" : grisé/désactivé tant qu'aucun verre n'est présent. */
    if (s_svc_serve_btn != NULL) {
        if (state->svc_test_glass_present) {
            lv_obj_clear_state(s_svc_serve_btn, LV_STATE_DISABLED);
        } else {
            lv_obj_add_state(s_svc_serve_btn, LV_STATE_DISABLED);
        }
    }

    /* Statut de la pompe. */
    if (s_svc_pump_status != NULL) {
        if (state->svc_test_pump_ms > 0U) {
            const unsigned int secs = ((unsigned int)state->svc_test_pump_ms + 999U) / 1000U;
            lv_label_set_text_fmt(s_svc_pump_status, "Pompe ACTIVE · %u s restantes", secs);
            lv_obj_set_style_text_color(s_svc_pump_status, ui_color_success(), 0);
        } else if (!state->svc_test_glass_present) {
            lv_label_set_text(s_svc_pump_status, "Pose un verre, puis touche « Vérifier le verre » ou « Tester ».");
            lv_obj_set_style_text_color(s_svc_pump_status, ui_color_text_muted(), 0);
        } else {
            lv_label_set_text(s_svc_pump_status, "Pompe arrêtée · prêt à tester.");
            lv_obj_set_style_text_color(s_svc_pump_status, ui_color_text_secondary(), 0);
        }
    }
}
