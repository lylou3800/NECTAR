#include "board.h"

#include "board_display.h"

static bool s_board_ready;

esp_err_t board_init(void)
{
    if (s_board_ready) {
        return ESP_OK;
    }

    ESP_ERROR_CHECK(board_display_init());
    s_board_ready = true;
    return ESP_OK;
}

bool board_lvgl_lock(int timeout_ms)
{
    return board_display_lock(timeout_ms);
}

void board_lvgl_unlock(void)
{
    board_display_unlock();
}

void board_backlight_set(bool enabled)
{
    board_display_backlight_set(enabled);
}

bool board_is_ready(void)
{
    return s_board_ready;
}