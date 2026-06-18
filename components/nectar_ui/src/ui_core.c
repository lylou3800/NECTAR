#include "ui_core.h"

#include "app_controller.h"
#include "board.h"
#include "ui_nav_controller.h"
#include "ui_state_model.h"
#include "ui_styles.h"

static lv_timer_t *s_ui_runtime_timer;

static void ui_core_runtime_tick(lv_timer_t *timer)
{
    const ui_state_model_t *state;
    bool refresh_requested;

    (void)timer;

    app_controller_tick(100);
    refresh_requested = ui_state_model_consume_refresh();
    state = ui_state_model_get();
    if (state->requested_view != ui_nav_current()) {
        ui_nav_navigate(state->requested_view);
    } else if (refresh_requested) {
        ui_nav_refresh_current();
    }
}

esp_err_t ui_core_init(void)
{
    if (!board_lvgl_lock(-1)) {
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(app_controller_init());
    ui_styles_init();
    ui_nav_controller_init();
    ui_nav_navigate(ui_state_model_get()->requested_view);
    s_ui_runtime_timer = lv_timer_create(ui_core_runtime_tick, 100, NULL);
    if (s_ui_runtime_timer == NULL) {
        board_lvgl_unlock();
        return ESP_ERR_NO_MEM;
    }

    board_lvgl_unlock();
    return ESP_OK;
}

void ui_core_task(void)
{
}