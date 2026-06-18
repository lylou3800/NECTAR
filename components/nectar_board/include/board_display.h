#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_DISPLAY_H_RES 800
#define BOARD_DISPLAY_V_RES 480

esp_err_t board_display_init(void);
bool board_display_lock(int timeout_ms);
void board_display_unlock(void);
void board_display_backlight_set(bool enabled);
uint16_t board_display_width(void);
uint16_t board_display_height(void);

#ifdef __cplusplus
}
#endif