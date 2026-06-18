#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void screen_boot_create(lv_obj_t *screen);
void screen_boot_refresh(void);

#ifdef __cplusplus
}
#endif