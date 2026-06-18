#pragma once

#include <stdbool.h>

#include "lvgl.h"
#include "ui_metrics.h"
#include "ui_styles.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    lv_obj_t *container;
    lv_obj_t *eyebrow;
    lv_obj_t *title;
    lv_obj_t *subtitle;
} ui_header_block_t;

static inline lv_obj_t *ui_create_button(lv_obj_t *parent,
                                         const char *label,
                                         lv_coord_t width,
                                         lv_coord_t height,
                                         lv_style_t *base_style,
                                         lv_style_t *pressed_style,
                                         lv_event_cb_t event_cb,
                                         void *user_data)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_t *title = lv_label_create(button);

    lv_obj_remove_style_all(button);
    lv_obj_add_style(button, base_style, 0);
    if (pressed_style != NULL) {
        lv_obj_add_style(button, pressed_style, LV_STATE_PRESSED);
    }
    lv_obj_add_style(button, ui_style_button_disabled(), LV_STATE_DISABLED);
    lv_obj_set_size(button, width, height);
    lv_obj_clear_flag(button, LV_OBJ_FLAG_SCROLLABLE);
    if (event_cb != NULL) {
        lv_obj_add_event_cb(button, event_cb, LV_EVENT_CLICKED, user_data);
    }

    lv_obj_set_width(title, width - 28);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_label_set_text(title, label);
    lv_obj_center(title);
    return button;
}

static inline lv_obj_t *ui_create_action_bar(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);

    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, UI_CONTENT_WIDTH, UI_ACTION_BAR_HEIGHT);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, ui_action_bar_y());
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    return bar;
}

static inline ui_header_block_t ui_create_screen_header(lv_obj_t *parent,
                                                        const char *eyebrow_text,
                                                        const char *title_text,
                                                        const char *subtitle_text,
                                                        bool centered)
{
    ui_header_block_t header = {0};
    lv_coord_t title_y = 0;
    lv_coord_t subtitle_width = centered ? 560 : 520;

    header.container = lv_obj_create(parent);
    lv_obj_remove_style_all(header.container);
    lv_obj_set_size(header.container, UI_CONTENT_WIDTH, UI_HEADER_HEIGHT);
    lv_obj_align(header.container, LV_ALIGN_TOP_MID, 0, UI_MARGIN_TOP);
    lv_obj_clear_flag(header.container, LV_OBJ_FLAG_SCROLLABLE);

    if ((eyebrow_text != NULL) && (eyebrow_text[0] != '\0')) {
        header.eyebrow = lv_label_create(header.container);
        lv_obj_add_style(header.eyebrow, ui_style_overline(), 0);
        lv_obj_set_width(header.eyebrow, UI_CONTENT_WIDTH);
        lv_obj_set_style_text_align(header.eyebrow, centered ? LV_TEXT_ALIGN_CENTER : LV_TEXT_ALIGN_LEFT, 0);
        lv_label_set_text(header.eyebrow, eyebrow_text);
        lv_obj_align(header.eyebrow, centered ? LV_ALIGN_TOP_MID : LV_ALIGN_TOP_LEFT, 0, 0);
        title_y = 18;
    }

    header.title = lv_label_create(header.container);
    lv_obj_add_style(header.title, ui_style_title(), 0);
    lv_obj_set_width(header.title, centered ? 620 : 540);
    lv_obj_set_style_text_align(header.title, centered ? LV_TEXT_ALIGN_CENTER : LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_long_mode(header.title, LV_LABEL_LONG_WRAP);
    lv_label_set_text(header.title, title_text);
    lv_obj_align(header.title, centered ? LV_ALIGN_TOP_MID : LV_ALIGN_TOP_LEFT, 0, title_y);

    if ((subtitle_text != NULL) && (subtitle_text[0] != '\0')) {
        header.subtitle = lv_label_create(header.container);
        lv_obj_add_style(header.subtitle, ui_style_body(), 0);
        lv_obj_set_width(header.subtitle, subtitle_width);
        lv_obj_set_style_text_align(header.subtitle, centered ? LV_TEXT_ALIGN_CENTER : LV_TEXT_ALIGN_LEFT, 0);
        lv_label_set_long_mode(header.subtitle, LV_LABEL_LONG_WRAP);
        lv_label_set_text(header.subtitle, subtitle_text);
        lv_obj_align_to(
            header.subtitle,
            header.title,
            centered ? LV_ALIGN_OUT_BOTTOM_MID : LV_ALIGN_OUT_BOTTOM_LEFT,
            0,
            8
        );
    }

    return header;
}

#ifdef __cplusplus
}
#endif