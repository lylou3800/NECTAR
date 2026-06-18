#pragma once

#include "lvgl.h"
#include "recipe_model.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t *ui_card_recipe_create(lv_obj_t *parent,
                                const recipe_model_t *recipe,
                                lv_event_cb_t event_cb,
                                void *user_data);

#ifdef __cplusplus
}
#endif