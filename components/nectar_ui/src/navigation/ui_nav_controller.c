#include "ui_nav_controller.h"

#include "screen_boot.h"
#include "screen_admin_auth.h"
#include "screen_custom_mix.h"
#include "screen_drink_detail.h"
#include "screen_error.h"
#include "screen_glass_check.h"
#include "screen_home.h"
#include "screen_maintenance.h"
#include "screen_preparing.h"
#include "screen_ready.h"
#include "screen_recipes.h"
#include "ui_styles.h"

static app_view_id_t s_current_screen = APP_VIEW_BOOT;
static bool s_has_loaded_screen;

void ui_nav_controller_init(void)
{
    s_has_loaded_screen = false;
}

app_view_id_t ui_nav_current(void)
{
    return s_current_screen;
}

void ui_nav_navigate(app_view_id_t screen_id)
{
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_add_style(screen, ui_style_screen(), 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    switch (screen_id) {
    case APP_VIEW_RECIPES:
        screen_recipes_create(screen);
        break;
    case APP_VIEW_DRINK_DETAIL:
        screen_drink_detail_create(screen);
        break;
    case APP_VIEW_CUSTOM_MIX:
        screen_custom_mix_create(screen);
        break;
    case APP_VIEW_GLASS_CHECK:
        screen_glass_check_create(screen);
        break;
    case APP_VIEW_PREPARING:
        screen_preparing_create(screen);
        break;
    case APP_VIEW_READY:
        screen_ready_create(screen);
        break;
    case APP_VIEW_ERROR:
        screen_error_create(screen);
        break;
    case APP_VIEW_ADMIN_AUTH:
        screen_admin_auth_create(screen);
        break;
    case APP_VIEW_MAINTENANCE:
        screen_maintenance_create(screen);
        break;
    case APP_VIEW_HOME:
        screen_home_create(screen);
        break;
    case APP_VIEW_BOOT:
    default:
        screen_boot_create(screen);
        break;
    }

    /* Resolve the full layout before the screen becomes active to avoid first-frame jumps. */
    lv_obj_update_layout(screen);
    s_current_screen = screen_id;

    if (!s_has_loaded_screen) {
        lv_scr_load(screen);
        s_has_loaded_screen = true;
        return;
    }

    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}

void ui_nav_refresh_current(void)
{
    switch (s_current_screen) {
    case APP_VIEW_BOOT:
        screen_boot_refresh();
        break;
    case APP_VIEW_HOME:
        screen_home_refresh();
        break;
    case APP_VIEW_RECIPES:
        screen_recipes_refresh();
        break;
    case APP_VIEW_DRINK_DETAIL:
        screen_drink_detail_refresh();
        break;
    case APP_VIEW_CUSTOM_MIX:
        screen_custom_mix_refresh();
        break;
    case APP_VIEW_GLASS_CHECK:
        screen_glass_check_refresh();
        break;
    case APP_VIEW_PREPARING:
        screen_preparing_refresh();
        break;
    case APP_VIEW_READY:
        screen_ready_refresh();
        break;
    case APP_VIEW_ERROR:
        screen_error_refresh();
        break;
    case APP_VIEW_ADMIN_AUTH:
        screen_admin_auth_refresh();
        break;
    case APP_VIEW_MAINTENANCE:
        screen_maintenance_refresh();
        break;
    default:
        break;
    }
}