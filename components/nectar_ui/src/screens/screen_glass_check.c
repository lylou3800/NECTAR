#include "screen_glass_check.h"

#include "ui_chrome.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_glass_guide.h"
#include "ui_styles.h"
#include "ui_state_model.h"

static lv_obj_t *s_glass_guide;
static lv_obj_t *s_glass_status;

void screen_glass_check_create(lv_obj_t *screen)
{
    ui_create_screen_header(
        screen,
        "STEP 1",
        "Place your glass",
        "The pour begins only after the placement ring confirms a clean, stable position.",
        false
    );

    s_glass_guide = ui_glass_guide_create(screen);
    lv_obj_align(s_glass_guide, LV_ALIGN_TOP_MID, 0, 118);

    s_glass_status = lv_label_create(screen);
    lv_obj_add_style(s_glass_status, ui_style_banner(), 0);
    lv_obj_set_width(s_glass_status, UI_CONTENT_WIDTH);
    lv_label_set_long_mode(s_glass_status, LV_LABEL_LONG_WRAP);
    lv_obj_align(s_glass_status, LV_ALIGN_BOTTOM_MID, 0, -22);

    screen_glass_check_refresh();
}

void screen_glass_check_refresh(void)
{
    const ui_state_model_t *state = ui_state_model_get();

    if ((s_glass_guide == NULL) || (s_glass_status == NULL)) {
        return;
    }

    ui_glass_guide_update(s_glass_guide, state->glass_detected);
    if (state->glass_detected) {
        lv_label_set_text(s_glass_status, "Sensor confirmed. Preparation can now begin with the glass safely locked in place.");
        lv_obj_set_style_border_color(s_glass_status, ui_color_success(), 0);
    } else {
        lv_label_set_text(s_glass_status, "Serving stays paused until the ring confirms placement. Keep the glass inside the guide and the next step will start automatically.");
        lv_obj_set_style_border_color(s_glass_status, ui_color_warning(), 0);
    }
}