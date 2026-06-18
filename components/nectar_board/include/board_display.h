#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "driver/i2c_master.h"
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
/* Bus I2C maitre partagé (GPIO 8/9, I2C_NUM_0) : écran tactile + CH422G, et
 * réutilisé par gpio.c pour le TCA9548A (pompe) et le MCP23017 (capteur verre).
 * Renvoie NULL tant que l'écran n'a pas été initialisé. */
i2c_master_bus_handle_t board_i2c_bus(void);
uint16_t board_display_width(void);
uint16_t board_display_height(void);

#ifdef __cplusplus
}
#endif