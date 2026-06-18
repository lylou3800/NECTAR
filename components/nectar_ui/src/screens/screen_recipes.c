#include "screen_recipes.h"

#include <stdint.h>

#include "app_controller.h"
#include "recipe_model.h"
#include "ui_card_recipe.h"
#include "ui_chrome.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"

#define UI_RECIPE_SHOWCASE_HEIGHT 248
#define UI_RECIPE_FOCUS_PANEL_WIDTH 282
#define UI_RECIPE_CAROUSEL_WIDTH (UI_CONTENT_WIDTH - UI_RECIPE_FOCUS_PANEL_WIDTH - UI_SECTION_GAP)

static lv_obj_t *s_recipe_focus_panel;
static lv_obj_t *s_recipe_name_label;
static lv_obj_t *s_recipe_tagline_label;
static lv_obj_t *s_recipe_status_label;
static lv_obj_t *s_recipe_meta_label;
static lv_obj_t *s_recipe_position_label;
static lv_obj_t *s_recipe_swipe_label;
static lv_obj_t *s_recipe_carousel;
static size_t s_recipe_active_index;

static const char *recipes_status_text(const recipe_model_t *recipe)
{
    if ((recipe == NULL) || !recipe->available) {
        return "Unavailable";
    }

    if (recipe->stock_percent <= 35U) {
        return "Low stock";
    }

    return "Ready to pour";
}

static void recipes_apply_focus_state(const recipe_model_t *recipe)
{
    lv_color_t border_color = ui_color_line();
    lv_color_t text_color = ui_color_text_secondary();

    if ((s_recipe_focus_panel == NULL) || (s_recipe_status_label == NULL) || (recipe == NULL)) {
        return;
    }

    if (!recipe->available) {
        border_color = ui_color_error();
        text_color = ui_color_error();
    } else if (recipe->stock_percent <= 35U) {
        border_color = ui_color_warning();
        text_color = ui_color_text_primary();
    }

    lv_obj_set_style_border_color(s_recipe_focus_panel, border_color, 0);
    lv_obj_set_style_outline_color(s_recipe_focus_panel, border_color, 0);
    lv_obj_set_style_border_color(s_recipe_status_label, border_color, 0);
    lv_obj_set_style_text_color(s_recipe_status_label, text_color, 0);
}

static void recipes_update_focus(size_t index)
{
    const size_t total = recipe_model_count();
    const recipe_model_t *recipe;

    if ((total == 0U) || (index >= total)) {
        return;
    }

    recipe = recipe_model_at(index);
    if (recipe == NULL) {
        return;
    }

    s_recipe_active_index = index;

    if (s_recipe_name_label != NULL) {
        lv_label_set_text(s_recipe_name_label, recipe->name);
    }

    if (s_recipe_tagline_label != NULL) {
        lv_label_set_text(s_recipe_tagline_label, recipe->tagline);
    }

    if (s_recipe_status_label != NULL) {
        lv_label_set_text(s_recipe_status_label, recipes_status_text(recipe));
    }

    if (s_recipe_meta_label != NULL) {
        lv_label_set_text_fmt(
            s_recipe_meta_label,
            "%u mL serve\n%u ingredient%s  |  stock %u%%",
            recipe->total_ml,
            recipe->ingredient_count,
            recipe->ingredient_count > 1U ? "s" : "",
            recipe->stock_percent
        );
    }

    if (s_recipe_position_label != NULL) {
        lv_label_set_text_fmt(
            s_recipe_position_label,
            "%02u / %02u",
            (unsigned int)(index + 1U),
            (unsigned int)total
        );
    }

    if (s_recipe_swipe_label != NULL) {
        lv_label_set_text_fmt(
            s_recipe_swipe_label,
            total > 1U ?
                "Swipe to browse %u signatures" :
                "Tap to open this signature",
            (unsigned int)total
        );
    }

    recipes_apply_focus_state(recipe);
}

static size_t recipes_scroll_index(void)
{
    const size_t total = recipe_model_count();
    const lv_coord_t step = UI_RECIPE_CARD_WIDTH + UI_RECIPE_GRID_GAP;
    lv_coord_t scroll_left;
    size_t index;

    if ((s_recipe_carousel == NULL) || (total == 0U) || (step <= 0)) {
        return 0U;
    }

    scroll_left = lv_obj_get_scroll_left(s_recipe_carousel);
    index = (size_t)((scroll_left + (step / 2)) / step);

    if (index >= total) {
        index = total - 1U;
    }

    return index;
}

static void recipes_carousel_scroll_end_cb(lv_event_t *event)
{
    (void)event;
    recipes_update_focus(recipes_scroll_index());
}

static void recipes_home_cb(lv_event_t *event)
{
    (void)event;
    app_controller_open_home();
}

static void recipes_custom_mix_cb(lv_event_t *event)
{
    (void)event;
    app_controller_open_custom_mix();
}

static void recipe_select_cb(lv_event_t *event)
{
    const uintptr_t recipe_index = (uintptr_t)lv_event_get_user_data(event);
    app_controller_select_recipe((size_t)recipe_index);
}

void screen_recipes_create(lv_obj_t *screen)
{
    lv_obj_t *actions;
    lv_obj_t *showcase;
    lv_obj_t *focus_eyebrow;
    lv_obj_t *focus_accent;
    lv_obj_t *focus_line;
    lv_obj_t *rail_shell;
    size_t index;

    s_recipe_focus_panel = NULL;
    s_recipe_name_label = NULL;
    s_recipe_tagline_label = NULL;
    s_recipe_status_label = NULL;
    s_recipe_meta_label = NULL;
    s_recipe_position_label = NULL;
    s_recipe_swipe_label = NULL;
    s_recipe_carousel = NULL;
    s_recipe_active_index = 0U;

    ui_create_screen_header(
        screen,
        "CELLAR MENU",
        "Signature cocktails",
        "Slide through the selection and open a card for full recipe details.",
        false
    );

    showcase = lv_obj_create(screen);
    lv_obj_remove_style_all(showcase);
    lv_obj_set_size(showcase, UI_CONTENT_WIDTH, UI_RECIPE_SHOWCASE_HEIGHT);
    lv_obj_align(showcase, LV_ALIGN_TOP_MID, 0, 118);
    lv_obj_clear_flag(showcase, LV_OBJ_FLAG_SCROLLABLE);

    s_recipe_focus_panel = lv_obj_create(showcase);
    lv_obj_remove_style_all(s_recipe_focus_panel);
    lv_obj_add_style(s_recipe_focus_panel, ui_style_card_inset(), 0);
    lv_obj_set_style_pad_all(s_recipe_focus_panel, 14, 0);
    lv_obj_set_size(s_recipe_focus_panel, UI_RECIPE_FOCUS_PANEL_WIDTH, UI_RECIPE_SHOWCASE_HEIGHT);
    lv_obj_align(s_recipe_focus_panel, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_clear_flag(s_recipe_focus_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(s_recipe_focus_panel, ui_color_surface(), 0);
    lv_obj_set_style_bg_grad_color(s_recipe_focus_panel, ui_color_surface_alt(), 0);
    lv_obj_set_style_border_color(s_recipe_focus_panel, ui_color_line(), 0);

    focus_accent = lv_obj_create(s_recipe_focus_panel);
    lv_obj_remove_style_all(focus_accent);
    lv_obj_set_size(focus_accent, 3, 94);
    lv_obj_align(focus_accent, LV_ALIGN_TOP_RIGHT, -4, 30);
    lv_obj_set_style_radius(focus_accent, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(focus_accent, ui_color_accent(), 0);
    lv_obj_set_style_bg_opa(focus_accent, LV_OPA_COVER, 0);

    focus_eyebrow = lv_label_create(s_recipe_focus_panel);
    lv_obj_add_style(focus_eyebrow, ui_style_overline(), 0);
    lv_label_set_text(focus_eyebrow, "CELLAR CURATION");
    lv_obj_align(focus_eyebrow, LV_ALIGN_TOP_LEFT, 0, 0);

    s_recipe_position_label = lv_label_create(s_recipe_focus_panel);
    lv_obj_add_style(s_recipe_position_label, ui_style_overline(), 0);
    lv_obj_set_style_text_align(s_recipe_position_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_text(s_recipe_position_label, "01 / 01");
    lv_obj_align(s_recipe_position_label, LV_ALIGN_TOP_RIGHT, 0, 0);

    focus_line = lv_obj_create(s_recipe_focus_panel);
    lv_obj_remove_style_all(focus_line);
    lv_obj_set_size(focus_line, 48, 2);
    lv_obj_align(focus_line, LV_ALIGN_TOP_LEFT, 0, 24);
    lv_obj_set_style_radius(focus_line, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(focus_line, ui_color_accent(), 0);
    lv_obj_set_style_bg_grad_color(focus_line, ui_color_accent_secondary(), 0);
    lv_obj_set_style_bg_grad_dir(focus_line, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_bg_opa(focus_line, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(focus_line, 0, 0);

    s_recipe_name_label = lv_label_create(s_recipe_focus_panel);
    lv_obj_add_style(s_recipe_name_label, ui_style_title(), 0);
    lv_obj_set_width(s_recipe_name_label, 220);
    lv_label_set_long_mode(s_recipe_name_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(s_recipe_name_label, "-");
    lv_obj_align(s_recipe_name_label, LV_ALIGN_TOP_LEFT, 0, 42);

    s_recipe_tagline_label = lv_label_create(s_recipe_focus_panel);
    lv_obj_add_style(s_recipe_tagline_label, ui_style_body(), 0);
    lv_obj_set_width(s_recipe_tagline_label, 220);
    lv_label_set_long_mode(s_recipe_tagline_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(s_recipe_tagline_label, "-");
    lv_obj_align_to(s_recipe_tagline_label, s_recipe_name_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    s_recipe_status_label = lv_label_create(s_recipe_focus_panel);
    lv_obj_add_style(s_recipe_status_label, ui_style_badge(), 0);
    lv_label_set_text(s_recipe_status_label, "Ready to pour");
    lv_obj_align(s_recipe_status_label, LV_ALIGN_BOTTOM_LEFT, 0, -84);

    s_recipe_meta_label = lv_label_create(s_recipe_focus_panel);
    lv_obj_add_style(s_recipe_meta_label, ui_style_caption(), 0);
    lv_obj_set_width(s_recipe_meta_label, 220);
    lv_label_set_long_mode(s_recipe_meta_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(s_recipe_meta_label, "-");
    lv_obj_align_to(s_recipe_meta_label, s_recipe_status_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    s_recipe_swipe_label = lv_label_create(s_recipe_focus_panel);
    lv_obj_add_style(s_recipe_swipe_label, ui_style_caption(), 0);
    lv_obj_set_width(s_recipe_swipe_label, 220);
    lv_label_set_long_mode(s_recipe_swipe_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(s_recipe_swipe_label, "Swipe sideways to browse the menu.");
    lv_obj_align(s_recipe_swipe_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    rail_shell = lv_obj_create(showcase);
    lv_obj_remove_style_all(rail_shell);
    lv_obj_add_style(rail_shell, ui_style_card(), 0);
    lv_obj_set_style_pad_all(rail_shell, 0, 0);
    lv_obj_set_size(rail_shell, UI_RECIPE_CAROUSEL_WIDTH, UI_RECIPE_SHOWCASE_HEIGHT);
    lv_obj_align(rail_shell, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_clear_flag(rail_shell, LV_OBJ_FLAG_SCROLLABLE);

    s_recipe_carousel = lv_obj_create(rail_shell);
    lv_obj_remove_style_all(s_recipe_carousel);
    lv_obj_set_size(s_recipe_carousel, UI_RECIPE_CAROUSEL_WIDTH, UI_RECIPE_SHOWCASE_HEIGHT);
    lv_obj_center(s_recipe_carousel);
    lv_obj_set_flex_flow(s_recipe_carousel, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_left(s_recipe_carousel, (UI_RECIPE_CAROUSEL_WIDTH - UI_RECIPE_CARD_WIDTH) / 2, 0);
    lv_obj_set_style_pad_right(s_recipe_carousel, (UI_RECIPE_CAROUSEL_WIDTH - UI_RECIPE_CARD_WIDTH) / 2, 0);
    lv_obj_set_style_pad_top(s_recipe_carousel, 0, 0);
    lv_obj_set_style_pad_bottom(s_recipe_carousel, 0, 0);
    lv_obj_set_style_pad_gap(s_recipe_carousel, UI_RECIPE_GRID_GAP, 0);
    lv_obj_set_scroll_dir(s_recipe_carousel, LV_DIR_HOR);
    lv_obj_set_scroll_snap_x(s_recipe_carousel, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(s_recipe_carousel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(s_recipe_carousel, LV_OBJ_FLAG_SCROLL_ONE);
    lv_obj_add_event_cb(s_recipe_carousel, recipes_carousel_scroll_end_cb, LV_EVENT_SCROLL_END, NULL);

    for (index = 0; index < recipe_model_count(); index++) {
        const recipe_model_t *recipe = recipe_model_at(index);
        ui_card_recipe_create(s_recipe_carousel, recipe, recipe_select_cb, (void *)(uintptr_t)index);
    }

    lv_obj_update_layout(s_recipe_carousel);
    if (recipe_model_count() > 0U) {
        lv_obj_scroll_to_view(lv_obj_get_child(s_recipe_carousel, 0), LV_ANIM_OFF);
        recipes_update_focus(0U);
    }

    actions = ui_create_action_bar(screen);
    lv_obj_align(ui_create_button(
        actions,
        "Home",
        UI_TERTIARY_BUTTON_WIDTH,
        UI_SECONDARY_BUTTON_HEIGHT,
        ui_style_button_secondary(),
        ui_style_button_secondary_pressed(),
        recipes_home_cb,
        NULL
    ), LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_align(ui_create_button(
        actions,
        "Craft blend",
        UI_PRIMARY_BUTTON_WIDTH,
        UI_PRIMARY_BUTTON_HEIGHT,
        ui_style_button_primary(),
        ui_style_button_primary_pressed(),
        recipes_custom_mix_cb,
        NULL
    ), LV_ALIGN_CENTER, 0, 0);
}

void screen_recipes_refresh(void)
{
    recipes_update_focus(recipes_scroll_index());
}