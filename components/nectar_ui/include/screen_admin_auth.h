#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void screen_admin_auth_create(lv_obj_t *screen);
void screen_admin_auth_refresh(void);

#ifdef __cplusplus
}
#endif