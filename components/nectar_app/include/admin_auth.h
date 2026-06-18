#pragma once

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t admin_auth_init(void);
bool admin_auth_verify_pin(const char *pin);
esp_err_t admin_auth_set_pin(const char *pin);

#ifdef __cplusplus
}
#endif