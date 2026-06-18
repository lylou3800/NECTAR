#include "admin_auth.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "nvs.h"

#define ADMIN_AUTH_NAMESPACE "nectar_admin"
#define ADMIN_AUTH_PIN_KEY "pin"

static char s_admin_pin[5] = "2468";
static bool s_admin_pin_loaded;

static bool admin_auth_pin_valid(const char *pin)
{
    size_t index;

    if (pin == NULL || strlen(pin) != 4) {
        return false;
    }

    for (index = 0; index < 4; index++) {
        if (!isdigit((unsigned char)pin[index])) {
            return false;
        }
    }

    return true;
}

esp_err_t admin_auth_set_pin(const char *pin)
{
    nvs_handle_t handle;
    esp_err_t err;

    if (!admin_auth_pin_valid(pin)) {
        return ESP_ERR_INVALID_ARG;
    }

    err = nvs_open(ADMIN_AUTH_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_str(handle, ADMIN_AUTH_PIN_KEY, pin);
    if (err != ESP_OK) {
        nvs_close(handle);
        return err;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        nvs_close(handle);
        return err;
    }

    memcpy(s_admin_pin, pin, sizeof(s_admin_pin));
    s_admin_pin_loaded = true;
    nvs_close(handle);
    return ESP_OK;
}

esp_err_t admin_auth_init(void)
{
    nvs_handle_t handle;
    size_t pin_size = sizeof(s_admin_pin);
    esp_err_t err;

    if (s_admin_pin_loaded) {
        return ESP_OK;
    }

    err = nvs_open(ADMIN_AUTH_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_get_str(handle, ADMIN_AUTH_PIN_KEY, s_admin_pin, &pin_size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(handle);
        return admin_auth_set_pin(s_admin_pin);
    }

    if (err != ESP_OK || !admin_auth_pin_valid(s_admin_pin)) {
        nvs_close(handle);
        return admin_auth_set_pin("2468");
    }

    s_admin_pin_loaded = true;
    nvs_close(handle);
    return ESP_OK;
}

bool admin_auth_verify_pin(const char *pin)
{
    if (!s_admin_pin_loaded && admin_auth_init() != ESP_OK) {
        return false;
    }

    return admin_auth_pin_valid(pin) && strcmp(pin, s_admin_pin) == 0;
}