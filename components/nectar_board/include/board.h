#pragma once

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t board_init(void);
bool board_lvgl_lock(int timeout_ms);
void board_lvgl_unlock(void);
void board_backlight_set(bool enabled);
bool board_is_ready(void);

#ifdef __cplusplus
}
#endif