#include "screen_preparing.h"

#include "ui_chrome.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_progress_panel.h"
#include "ui_styles.h"
#include "ui_state_model.h"

static lv_obj_t *s_progress_panel;

void screen_preparing_create(lv_obj_t *screen)
{
    lv_obj_t *hint;

    ui_create_screen_header(
        screen,
        "ÉTAPE 2",
        "On prépare ton verre",
        "Encore quelques instants…",
        false
    );

    s_progress_panel = ui_progress_panel_create(screen);
    lv_obj_align(s_progress_panel, LV_ALIGN_TOP_MID, 0, 118);

    hint = lv_label_create(screen);
    lv_obj_add_style(hint, ui_style_banner(), 0);
    lv_obj_set_width(hint, UI_CONTENT_WIDTH);
    lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
    lv_label_set_text(hint, "Garde ton verre en place, ta boisson arrive.");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -22);

    screen_preparing_refresh();
}

void screen_preparing_refresh(void)
{
    const ui_state_model_t *state = ui_state_model_get();
    ui_progress_panel_update(s_progress_panel, state->prepare_progress, state->prepare_step);
}