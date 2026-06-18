#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_VIEW_BOOT = 0,
    APP_VIEW_HOME,
    APP_VIEW_RECIPES,
    APP_VIEW_DRINK_DETAIL,
    APP_VIEW_CUSTOM_MIX,
    APP_VIEW_GLASS_CHECK,
    APP_VIEW_PREPARING,
    APP_VIEW_READY,
    APP_VIEW_ERROR,
    APP_VIEW_ADMIN_AUTH,
    APP_VIEW_MAINTENANCE,
} app_view_id_t;

typedef enum {
    APP_MACHINE_IDLE = 0,
    APP_MACHINE_WAITING_FOR_GLASS,
    APP_MACHINE_GLASS_DETECTED,
    APP_MACHINE_PREPARING,
    APP_MACHINE_READY,
    APP_MACHINE_UNAVAILABLE,
    APP_MACHINE_ERROR,
    APP_MACHINE_MAINTENANCE,
} app_machine_state_t;

typedef enum {
    APP_SELECTION_NONE = 0,
    APP_SELECTION_RECIPE,
    APP_SELECTION_CUSTOM,
} app_selection_kind_t;

typedef struct {
    app_view_id_t requested_view;
    app_machine_state_t machine_state;
    app_selection_kind_t selection_kind;
    int selected_recipe_index;
    uint16_t custom_mix_ml[3];
    uint16_t custom_total_ml;
    bool glass_detected;
    uint8_t prepare_progress;
    const char *prepare_step;
    bool ingredient_low_warning;
    const char *error_title;
    const char *error_message;
    bool admin_authenticated;
    char admin_pin_input[5];
    uint8_t admin_pin_length;
    const char *admin_pin_status;
    uint32_t state_elapsed_ms;
    bool svc_test_glass_present;   /* simulateur de test (espace service) : verre présent ? */
    uint16_t svc_test_pump_ms;     /* simulateur de test : ms restantes de pompe active */
    bool refresh_requested;
} ui_state_model_t;

void ui_state_model_reset(void);
const ui_state_model_t *ui_state_model_get(void);
ui_state_model_t *ui_state_model_mutable(void);
void ui_state_model_request_refresh(void);
bool ui_state_model_consume_refresh(void);

#ifdef __cplusplus
}
#endif