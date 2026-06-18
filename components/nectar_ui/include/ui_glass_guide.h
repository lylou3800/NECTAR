#pragma once

#include <stdbool.h>

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t *ui_glass_guide_create(lv_obj_t *parent);
void ui_glass_guide_update(lv_obj_t *guide, bool detected);

#ifdef __cplusplus
}
#endif