#include "screen_admin_auth.h"

#include <stdint.h>
#include <string.h>

#include "app_controller.h"
#include "ui_chrome.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_state_model.h"

static lv_obj_t *s_pin_dots;
static lv_obj_t *s_status_label;

static void admin_auth_apply_state(void)
{
    const ui_state_model_t *state = ui_state_model_get();
    char pin_dots[16] = "_ _ _ _";
    uint8_t index;

    if ((s_pin_dots == NULL) || (s_status_label == NULL)) {
        return;
    }

    for (index = 0; index < state->admin_pin_length && index < 4; index++) {
        pin_dots[index * 2] = '*';
    }

    lv_label_set_text(s_pin_dots, pin_dots);
    lv_label_set_text(s_status_label, state->admin_pin_status);
    lv_obj_set_style_text_color(
        s_status_label,
        strstr(state->admin_pin_status, "Incorrect") != NULL ? lv_color_hex(0xFF8E7A) : ui_color_text_secondary(),
        0
    );
}

static void admin_auth_digit_cb(lv_event_t *event)
{
    const uintptr_t digit = (uintptr_t)lv_event_get_user_data(event);

    app_controller_admin_pin_append((uint8_t)digit);
    if (ui_state_model_get()->requested_view == APP_VIEW_ADMIN_AUTH) {
        admin_auth_apply_state();
    }
}

static void admin_auth_backspace_cb(lv_event_t *event)
{
    (void)event;
    app_controller_admin_pin_backspace();
    if (ui_state_model_get()->requested_view == APP_VIEW_ADMIN_AUTH) {
        admin_auth_apply_state();
    }
}

static void admin_auth_submit_cb(lv_event_t *event)
{
    (void)event;
    app_controller_admin_pin_submit();
    if (ui_state_model_get()->requested_view == APP_VIEW_ADMIN_AUTH) {
        admin_auth_apply_state();
    }
}

static void admin_auth_cancel_cb(lv_event_t *event)
{
    (void)event;
    app_controller_open_home();
}

static lv_obj_t *admin_auth_create_key(lv_obj_t *parent,
                                       const char *text,
                                       lv_event_cb_t callback,
                                       void *user_data)
{
    return ui_create_button(
        parent,
        text,
        108,
        66,
        ui_style_button_secondary(),
        ui_style_button_secondary_pressed(),
        callback,
        user_data
    );
}

void screen_admin_auth_create(lv_obj_t *screen)
{
    lv_obj_t *shell;
    lv_obj_t *title;
    lv_obj_t *subtitle;
    lv_obj_t *pin_card;
    lv_obj_t *keypad;
    lv_obj_t *actions;

    ui_create_screen_header(
        screen,
        "SERVICE ACCESS",
        "Unlock diagnostics",
        "Enter the service PIN to access diagnostics and maintenance tools.",
        false
    );

    shell = lv_obj_create(screen);
    lv_obj_add_style(shell, ui_style_card(), 0);
    lv_obj_set_size(shell, UI_CONTENT_WIDTH, 252);
    lv_obj_align(shell, LV_ALIGN_TOP_MID, 0, 118);
    lv_obj_set_style_bg_color(shell, ui_color_surface_alt(), 0);
    lv_obj_set_style_bg_grad_color(shell, ui_color_surface_highlight(), 0);

    title = lv_label_create(shell);
    lv_obj_add_style(title, ui_style_overline(), 0);
    lv_label_set_text(title, "AUTHORIZED STAFF");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    subtitle = lv_label_create(shell);
    lv_obj_add_style(subtitle, ui_style_body(), 0);
    lv_obj_set_width(subtitle, 276);
    lv_label_set_long_mode(subtitle, LV_LABEL_LONG_WRAP);
    lv_label_set_text(subtitle, "Use the keypad to unlock service mode. Public ordering remains separate from this space.");
    lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    pin_card = lv_obj_create(shell);
    lv_obj_add_style(pin_card, ui_style_card_inset(), 0);
    lv_obj_set_size(pin_card, 286, 132);
    lv_obj_align(pin_card, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(pin_card, ui_color_surface_overlay(), 0);

    title = lv_label_create(pin_card);
    lv_obj_add_style(title, ui_style_overline(), 0);
    lv_label_set_text(title, "PIN");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    s_pin_dots = lv_label_create(pin_card);
    lv_obj_add_style(s_pin_dots, ui_style_title(), 0);
    lv_label_set_text(s_pin_dots, "_ _ _ _");
    lv_obj_align(s_pin_dots, LV_ALIGN_CENTER, 0, -8);

    s_status_label = lv_label_create(pin_card);
    lv_obj_add_style(s_status_label, ui_style_caption(), 0);
    lv_obj_set_width(s_status_label, 230);
    lv_label_set_long_mode(s_status_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(s_status_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    keypad = lv_obj_create(shell);
    lv_obj_remove_style_all(keypad);
    lv_obj_set_size(keypad, 348, 220);
    lv_obj_align(keypad, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_flex_flow(keypad, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_gap(keypad, 12, 0);
    lv_obj_clear_flag(keypad, LV_OBJ_FLAG_SCROLLABLE);

    admin_auth_create_key(keypad, "1", admin_auth_digit_cb, (void *)(uintptr_t)1);
    admin_auth_create_key(keypad, "2", admin_auth_digit_cb, (void *)(uintptr_t)2);
    admin_auth_create_key(keypad, "3", admin_auth_digit_cb, (void *)(uintptr_t)3);
    admin_auth_create_key(keypad, "4", admin_auth_digit_cb, (void *)(uintptr_t)4);
    admin_auth_create_key(keypad, "5", admin_auth_digit_cb, (void *)(uintptr_t)5);
    admin_auth_create_key(keypad, "6", admin_auth_digit_cb, (void *)(uintptr_t)6);
    admin_auth_create_key(keypad, "7", admin_auth_digit_cb, (void *)(uintptr_t)7);
    admin_auth_create_key(keypad, "8", admin_auth_digit_cb, (void *)(uintptr_t)8);
    admin_auth_create_key(keypad, "9", admin_auth_digit_cb, (void *)(uintptr_t)9);
    admin_auth_create_key(keypad, "Cancel", admin_auth_cancel_cb, NULL);
    admin_auth_create_key(keypad, "0", admin_auth_digit_cb, (void *)(uintptr_t)0);
    admin_auth_create_key(keypad, "Delete", admin_auth_backspace_cb, NULL);

    actions = ui_create_action_bar(screen);
    lv_obj_align(ui_create_button(
        actions,
        "Cancel",
        UI_SECONDARY_BUTTON_WIDTH,
        UI_SECONDARY_BUTTON_HEIGHT,
        ui_style_button_secondary(),
        ui_style_button_secondary_pressed(),
        admin_auth_cancel_cb,
        NULL
    ), LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_align(ui_create_button(
        actions,
        "Unlock",
        UI_PRIMARY_BUTTON_WIDTH,
        UI_PRIMARY_BUTTON_HEIGHT,
        ui_style_button_primary(),
        ui_style_button_primary_pressed(),
        admin_auth_submit_cb,
        NULL
    ), LV_ALIGN_RIGHT_MID, 0, 0);

    screen_admin_auth_refresh();
}

void screen_admin_auth_refresh(void)
{
    admin_auth_apply_state();
}