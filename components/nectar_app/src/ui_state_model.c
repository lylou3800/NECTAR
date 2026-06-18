#include "ui_state_model.h"

#include <string.h>

static ui_state_model_t s_state;

void ui_state_model_reset(void)
{
    memset(&s_state, 0, sizeof(s_state));
    s_state.requested_view = APP_VIEW_BOOT;
    s_state.machine_state = APP_MACHINE_IDLE;
    s_state.selection_kind = APP_SELECTION_NONE;
    s_state.selected_recipe_index = -1;
    s_state.prepare_step = "";
    s_state.error_title = "";
    s_state.error_message = "";
    s_state.admin_pin_input[0] = '\0';
    s_state.admin_pin_status = "Saisir le code PIN admin à 4 chiffres.";
    s_state.refresh_requested = true;
}

const ui_state_model_t *ui_state_model_get(void)
{
    return &s_state;
}

ui_state_model_t *ui_state_model_mutable(void)
{
    return &s_state;
}

void ui_state_model_request_refresh(void)
{
    s_state.refresh_requested = true;
}

bool ui_state_model_consume_refresh(void)
{
    const bool refresh_requested = s_state.refresh_requested;

    s_state.refresh_requested = false;
    return refresh_requested;
}