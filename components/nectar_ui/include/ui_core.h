#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ui_core_init(void);
void ui_core_task(void);

#ifdef __cplusplus
}
#endif