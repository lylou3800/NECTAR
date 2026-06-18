#pragma once

#include "ui_state_model.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_nav_controller_init(void);
void ui_nav_navigate(app_view_id_t screen_id);
void ui_nav_refresh_current(void);
app_view_id_t ui_nav_current(void);

#ifdef __cplusplus
}
#endif