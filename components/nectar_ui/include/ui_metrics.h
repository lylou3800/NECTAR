#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_SCREEN_WIDTH 800
#define UI_SCREEN_HEIGHT 480

#define UI_MARGIN_X 28
#define UI_MARGIN_TOP 22
#define UI_MARGIN_BOTTOM 22

#define UI_CONTENT_WIDTH (UI_SCREEN_WIDTH - (2 * UI_MARGIN_X))
/* Hauteur du conteneur d'en-tête : doit englober eyebrow + titre (police display)
 * + sous-titre, sinon LVGL clippe le bas du sous-titre (jambages coupés). */
#define UI_HEADER_HEIGHT 98
#define UI_ACTION_BAR_HEIGHT 72
#define UI_SECTION_GAP 18
#define UI_CARD_RADIUS 26

#define UI_PRIMARY_BUTTON_HEIGHT 60
#define UI_SECONDARY_BUTTON_HEIGHT 60
#define UI_PRIMARY_BUTTON_WIDTH 252
#define UI_SECONDARY_BUTTON_WIDTH 172
#define UI_TERTIARY_BUTTON_WIDTH 148

#define UI_RECIPE_CARD_WIDTH 196
#define UI_RECIPE_CARD_HEIGHT 244
#define UI_RECIPE_GRID_GAP 16

#define UI_METRIC_CARD_WIDTH 224
#define UI_METRIC_CARD_HEIGHT 118

#define UI_DETAIL_PANEL_HEIGHT 288
#define UI_MIX_PANEL_HEIGHT 284
#define UI_SUMMARY_PANEL_HEIGHT 286

static inline lv_coord_t ui_action_bar_y(void)
{
    return UI_SCREEN_HEIGHT - UI_MARGIN_BOTTOM - UI_ACTION_BAR_HEIGHT;
}

static inline lv_coord_t ui_content_top_y(void)
{
    return UI_MARGIN_TOP + UI_HEADER_HEIGHT;
}

static inline lv_coord_t ui_content_bottom_limit(void)
{
    return ui_action_bar_y() - UI_SECTION_GAP;
}

#ifdef __cplusplus
}
#endif