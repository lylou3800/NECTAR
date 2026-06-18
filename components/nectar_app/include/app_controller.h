#pragma once

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "recipe_model.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t app_controller_init(void);
void app_controller_tick(uint32_t delta_ms);

void app_controller_open_home(void);
void app_controller_open_recipes(void);
void app_controller_open_custom_mix(void);
void app_controller_select_recipe(size_t recipe_index);
void app_controller_back(void);
void app_controller_confirm_selection(void);
void app_controller_start_service(void);
void app_controller_acknowledge_ready(void);
void app_controller_open_admin_auth(void);
void app_controller_admin_pin_append(uint8_t digit);
void app_controller_admin_pin_backspace(void);
void app_controller_admin_pin_submit(void);
void app_controller_admin_logout(void);
void app_controller_enter_maintenance(void);
void app_controller_leave_maintenance(void);
void app_controller_update_custom_mix(size_t ingredient_index, uint16_t amount_ml);

/* Simulateur de test (espace service) */
void app_controller_svc_toggle_glass(void);
void app_controller_svc_start_pump(void);

const recipe_model_t *app_controller_selected_recipe(void);

#ifdef __cplusplus
}
#endif