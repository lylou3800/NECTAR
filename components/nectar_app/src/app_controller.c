#include "app_controller.h"

#include <string.h>

#include "admin_auth.h"
#include "app_services.h"
#include "drink_model.h"
#include "ui_state_model.h"

static void app_controller_reset_admin_auth(ui_state_model_t *state, const char *status)
{
    memset(state->admin_pin_input, 0, sizeof(state->admin_pin_input));
    state->admin_pin_length = 0;
    state->admin_pin_status = status;
}

static void app_controller_update_custom_total(ui_state_model_t *state)
{
    size_t index;

    state->custom_total_ml = 0;
    state->ingredient_low_warning = false;
    for (index = 0; index < 3; index++) {
        const drink_model_t *drink = drink_model_at(index);
        state->custom_total_ml += state->custom_mix_ml[index];
        if ((drink != NULL) && (drink->stock_percent <= 30) && (state->custom_mix_ml[index] > 0)) {
            state->ingredient_low_warning = true;
        }
    }
}

static void app_controller_set_view(ui_state_model_t *state, app_view_id_t view)
{
    state->requested_view = view;
    state->state_elapsed_ms = 0;
    ui_state_model_request_refresh();
}

static void app_controller_go_to_error(const char *title, const char *message)
{
    ui_state_model_t *state = ui_state_model_mutable();

    state->machine_state = APP_MACHINE_ERROR;
    state->error_title = title;
    state->error_message = message;
    app_controller_set_view(state, APP_VIEW_ERROR);
}

esp_err_t app_controller_init(void)
{
    size_t index;
    ui_state_model_t *state;

    ui_state_model_reset();
    ESP_ERROR_CHECK(admin_auth_init());
    app_services_init();

    state = ui_state_model_mutable();
    for (index = 0; index < 3; index++) {
        const drink_model_t *drink = drink_model_at(index);
        state->custom_mix_ml[index] = (drink != NULL) ? drink->default_ml : 0;
    }
    app_controller_update_custom_total(state);
    app_controller_reset_admin_auth(state, "Entre le code à 4 chiffres.");
    ui_state_model_request_refresh();
    return ESP_OK;
}

const recipe_model_t *app_controller_selected_recipe(void)
{
    const ui_state_model_t *state = ui_state_model_get();

    if (state->selected_recipe_index < 0) {
        return NULL;
    }

    return recipe_model_at((size_t)state->selected_recipe_index);
}

void app_controller_open_home(void)
{
    ui_state_model_t *state = ui_state_model_mutable();

    state->machine_state = APP_MACHINE_IDLE;
    app_controller_set_view(state, APP_VIEW_HOME);
}

void app_controller_open_recipes(void)
{
    ui_state_model_t *state = ui_state_model_mutable();
    app_controller_set_view(state, APP_VIEW_RECIPES);
}

void app_controller_open_custom_mix(void)
{
    ui_state_model_t *state = ui_state_model_mutable();

    state->selection_kind = APP_SELECTION_CUSTOM;
    app_controller_update_custom_total(state);
    app_controller_set_view(state, APP_VIEW_CUSTOM_MIX);
}

void app_controller_select_recipe(size_t recipe_index)
{
    const recipe_model_t *recipe = recipe_model_at(recipe_index);
    ui_state_model_t *state = ui_state_model_mutable();

    if (recipe == NULL) {
        app_controller_go_to_error("Cocktail introuvable", "Ce cocktail n'existe pas.");
        return;
    }

    if (!recipe->available) {
        state->machine_state = APP_MACHINE_UNAVAILABLE;
        state->error_title = "Cocktail indisponible";
        state->error_message = "Un ingrédient est épuisé. Choisis un autre cocktail.";
        app_controller_set_view(state, APP_VIEW_ERROR);
        return;
    }

    state->selection_kind = APP_SELECTION_RECIPE;
    state->selected_recipe_index = (int)recipe_index;
    state->ingredient_low_warning = recipe->stock_percent <= 35;
    app_controller_set_view(state, APP_VIEW_DRINK_DETAIL);
}

void app_controller_update_custom_mix(size_t ingredient_index, uint16_t amount_ml)
{
    ui_state_model_t *state = ui_state_model_mutable();
    const drink_model_t *drink = drink_model_at(ingredient_index);

    if ((drink == NULL) || (ingredient_index >= 3)) {
        return;
    }

    if (amount_ml < drink->min_ml) {
        amount_ml = drink->min_ml;
    }
    if (amount_ml > drink->max_ml) {
        amount_ml = drink->max_ml;
    }

    state->custom_mix_ml[ingredient_index] = amount_ml;
    app_controller_update_custom_total(state);
    if (state->requested_view != APP_VIEW_CUSTOM_MIX) {
        ui_state_model_request_refresh();
    }
}

void app_controller_confirm_selection(void)
{
    const ui_state_model_t *state = ui_state_model_get();

    if (state->selection_kind == APP_SELECTION_NONE) {
        app_controller_go_to_error("Fais ton choix", "Choisis un cocktail ou compose ton mélange.");
        return;
    }

    app_controller_start_service();
}

void app_controller_start_service(void)
{
    ui_state_model_t *state = ui_state_model_mutable();

    state->machine_state = APP_MACHINE_WAITING_FOR_GLASS;
    state->glass_detected = false;
    state->prepare_progress = 0;
    state->prepare_step = "En attente du verre";
    app_services_reset_glass_sequence();
    app_controller_set_view(state, APP_VIEW_GLASS_CHECK);
}

void app_controller_acknowledge_ready(void)
{
    app_controller_open_home();
}

void app_controller_open_admin_auth(void)
{
    ui_state_model_t *state = ui_state_model_mutable();

    app_controller_reset_admin_auth(state, "Entre le code à 4 chiffres.");
    app_controller_set_view(state, APP_VIEW_ADMIN_AUTH);
}

void app_controller_admin_pin_append(uint8_t digit)
{
    ui_state_model_t *state = ui_state_model_mutable();

    if (digit > 9 || state->admin_pin_length >= 4) {
        return;
    }

    state->admin_pin_input[state->admin_pin_length++] = (char)('0' + digit);
    state->admin_pin_input[state->admin_pin_length] = '\0';
    state->admin_pin_status = "Appuie sur Déverrouiller.";
    if (state->requested_view != APP_VIEW_ADMIN_AUTH) {
        ui_state_model_request_refresh();
    }
}

void app_controller_admin_pin_backspace(void)
{
    ui_state_model_t *state = ui_state_model_mutable();

    if (state->admin_pin_length == 0) {
        return;
    }

    state->admin_pin_length--;
    state->admin_pin_input[state->admin_pin_length] = '\0';
    state->admin_pin_status = "Entre le code à 4 chiffres.";
    if (state->requested_view != APP_VIEW_ADMIN_AUTH) {
        ui_state_model_request_refresh();
    }
}

void app_controller_admin_pin_submit(void)
{
    ui_state_model_t *state = ui_state_model_mutable();

    if (state->admin_pin_length != 4) {
        state->admin_pin_status = "Le code fait 4 chiffres.";
        if (state->requested_view != APP_VIEW_ADMIN_AUTH) {
            ui_state_model_request_refresh();
        }
        return;
    }

    if (!admin_auth_verify_pin(state->admin_pin_input)) {
        app_controller_reset_admin_auth(state, "Code incorrect. Réessaie.");
        if (state->requested_view != APP_VIEW_ADMIN_AUTH) {
            ui_state_model_request_refresh();
        }
        return;
    }

    state->admin_authenticated = true;
    state->machine_state = APP_MACHINE_MAINTENANCE;
    app_controller_reset_admin_auth(state, "Accès autorisé.");
    app_controller_set_view(state, APP_VIEW_MAINTENANCE);
}

void app_controller_admin_logout(void)
{
    ui_state_model_t *state = ui_state_model_mutable();

    state->admin_authenticated = false;
    app_controller_reset_admin_auth(state, "Entre le code à 4 chiffres.");
    app_controller_open_home();
}

void app_controller_enter_maintenance(void)
{
    ui_state_model_t *state = ui_state_model_mutable();

    if (!state->admin_authenticated) {
        app_controller_open_admin_auth();
        return;
    }

    state->machine_state = APP_MACHINE_MAINTENANCE;
    app_controller_set_view(state, APP_VIEW_MAINTENANCE);
}

void app_controller_leave_maintenance(void)
{
    app_controller_admin_logout();
}

void app_controller_back(void)
{
    const ui_state_model_t *state = ui_state_model_get();

    switch (state->requested_view) {
    case APP_VIEW_RECIPES:
    case APP_VIEW_CUSTOM_MIX:
    case APP_VIEW_ADMIN_AUTH:
    case APP_VIEW_MAINTENANCE:
    case APP_VIEW_ERROR:
        app_controller_open_home();
        break;
    case APP_VIEW_DRINK_DETAIL:
        app_controller_open_recipes();
        break;
    default:
        app_controller_open_home();
        break;
    }
}

void app_controller_tick(uint32_t delta_ms)
{
    ui_state_model_t *state = ui_state_model_mutable();

    switch (state->requested_view) {
    case APP_VIEW_BOOT:
        state->state_elapsed_ms += delta_ms;
        if (state->state_elapsed_ms >= 1400) {
            app_controller_open_home();
        } else {
            ui_state_model_request_refresh();
        }
        break;
    case APP_VIEW_GLASS_CHECK:
        app_services_tick(delta_ms);
        if (!state->glass_detected) {
            const bool glass_detected = app_services_glass_detected();

            if (glass_detected != state->glass_detected) {
                state->glass_detected = glass_detected;
            }

            if (state->glass_detected) {
                state->machine_state = APP_MACHINE_GLASS_DETECTED;
                state->state_elapsed_ms = 0;
                ui_state_model_request_refresh();
            }
        } else {
            state->state_elapsed_ms += delta_ms;
            if (state->state_elapsed_ms >= 700) {
                state->machine_state = APP_MACHINE_PREPARING;
                state->prepare_progress = 0;
                state->prepare_step = "Préparation";
                app_services_reset_prepare_sequence();
                app_controller_set_view(state, APP_VIEW_PREPARING);
            }
        }
        break;
    case APP_VIEW_PREPARING:
    {
        const uint8_t previous_progress = state->prepare_progress;
        const char *previous_step = state->prepare_step;

        app_services_tick(delta_ms);
        state->prepare_progress = app_services_prepare_progress();
        state->prepare_step = app_services_prepare_step();
        if ((state->prepare_progress != previous_progress) || (state->prepare_step != previous_step)) {
            ui_state_model_request_refresh();
        }
        if (app_services_prepare_complete()) {
            state->machine_state = APP_MACHINE_READY;
            app_controller_set_view(state, APP_VIEW_READY);
        }
        break;
    }
    case APP_VIEW_READY:
        state->state_elapsed_ms += delta_ms;
        if (state->state_elapsed_ms >= 2600) {
            app_controller_acknowledge_ready();
        }
        break;
    default:
        break;
    }
}