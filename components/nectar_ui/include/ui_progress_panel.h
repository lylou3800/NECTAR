#pragma once

#include <stdint.h>

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t *ui_progress_panel_create(lv_obj_t *parent);
void ui_progress_panel_update(lv_obj_t *panel, uint8_t progress, const char *step_text);

#ifdef __cplusplus
}
#endif