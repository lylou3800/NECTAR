#include "ui_allergen_badge.h"

#include "ui_styles.h"

lv_obj_t *ui_allergen_badge_create(lv_obj_t *parent, const char *text)
{
    lv_obj_t *badge = lv_label_create(parent);

    lv_obj_add_style(badge, ui_style_badge(), 0);
    lv_label_set_text(badge, text);
    return badge;
}