#include "ui_styles.h"

#include "ui_palette.h"
#include "ui_typography.h"

static lv_style_t s_style_screen;
static lv_style_t s_style_card;
static lv_style_t s_style_card_highlight;
static lv_style_t s_style_card_inset;
static lv_style_t s_style_button_primary;
static lv_style_t s_style_button_primary_pressed;
static lv_style_t s_style_button_secondary;
static lv_style_t s_style_button_secondary_pressed;
static lv_style_t s_style_button_disabled;
static lv_style_t s_style_title;
static lv_style_t s_style_heading;
static lv_style_t s_style_body;
static lv_style_t s_style_caption;
static lv_style_t s_style_overline;
static lv_style_t s_style_banner;
static lv_style_t s_style_banner_warning;
static lv_style_t s_style_banner_error;
static lv_style_t s_style_badge;
static lv_style_t s_style_badge_alert;
static lv_style_transition_dsc_t s_button_transition;
static bool s_styles_ready;

static const lv_style_prop_t s_button_transition_props[] = {
    LV_STYLE_BG_COLOR, LV_STYLE_BG_GRAD_COLOR, LV_STYLE_BORDER_COLOR,
    LV_STYLE_SHADOW_WIDTH, LV_STYLE_SHADOW_OPA, 0
};

void ui_styles_init(void)
{
    if (s_styles_ready) {
        return;
    }

    lv_style_transition_dsc_init(&s_button_transition, s_button_transition_props,
                                 lv_anim_path_ease_out, 90, 0, NULL);

    /* Écran : fond blanc chaud uni (pas de dégradé sombre). */
    lv_style_init(&s_style_screen);
    lv_style_set_bg_color(&s_style_screen, ui_color_background());
    lv_style_set_bg_opa(&s_style_screen, LV_OPA_COVER);
    lv_style_set_pad_all(&s_style_screen, 0);
    lv_style_set_border_width(&s_style_screen, 0);
    lv_style_set_text_color(&s_style_screen, ui_color_text_primary());
    lv_style_set_text_font(&s_style_screen, ui_font_body());

    /* Carte : surface blanche, bord clair, ombre douce. */
    lv_style_init(&s_style_card);
    lv_style_set_bg_color(&s_style_card, ui_color_surface());
    lv_style_set_bg_opa(&s_style_card, LV_OPA_COVER);
    lv_style_set_radius(&s_style_card, 20);
    lv_style_set_pad_all(&s_style_card, 16);
    lv_style_set_border_width(&s_style_card, 1);
    lv_style_set_border_color(&s_style_card, ui_color_line());
    lv_style_set_shadow_width(&s_style_card, 16);
    lv_style_set_shadow_opa(&s_style_card, LV_OPA_20);
    lv_style_set_shadow_color(&s_style_card, lv_color_hex(0xC9B7A6));
    lv_style_set_shadow_ofs_y(&s_style_card, 6);
    lv_style_set_shadow_spread(&s_style_card, 0);

    /* Carte mise en avant : bord orange. */
    lv_style_init(&s_style_card_highlight);
    lv_style_set_bg_color(&s_style_card_highlight, ui_color_surface());
    lv_style_set_bg_opa(&s_style_card_highlight, LV_OPA_COVER);
    lv_style_set_border_width(&s_style_card_highlight, 2);
    lv_style_set_border_color(&s_style_card_highlight, ui_color_accent());
    lv_style_set_shadow_width(&s_style_card_highlight, 22);
    lv_style_set_shadow_opa(&s_style_card_highlight, LV_OPA_30);
    lv_style_set_shadow_color(&s_style_card_highlight, ui_color_accent_glow());
    lv_style_set_shadow_ofs_y(&s_style_card_highlight, 8);

    /* Carte insérée : surface très douce. */
    lv_style_init(&s_style_card_inset);
    lv_style_set_bg_color(&s_style_card_inset, ui_color_surface_alt());
    lv_style_set_bg_opa(&s_style_card_inset, LV_OPA_COVER);
    lv_style_set_radius(&s_style_card_inset, 16);
    lv_style_set_pad_all(&s_style_card_inset, 14);
    lv_style_set_border_width(&s_style_card_inset, 1);
    lv_style_set_border_color(&s_style_card_inset, ui_color_line());
    lv_style_set_shadow_width(&s_style_card_inset, 0);

    /* Bouton principal : dégradé orange, texte blanc. */
    lv_style_init(&s_style_button_primary);
    lv_style_set_bg_color(&s_style_button_primary, ui_color_accent());
    lv_style_set_bg_grad_color(&s_style_button_primary, ui_color_accent_secondary());
    lv_style_set_bg_grad_dir(&s_style_button_primary, LV_GRAD_DIR_HOR);
    lv_style_set_bg_opa(&s_style_button_primary, LV_OPA_COVER);
    lv_style_set_radius(&s_style_button_primary, 26);
    lv_style_set_border_width(&s_style_button_primary, 0);
    lv_style_set_text_color(&s_style_button_primary, ui_color_text_ink());
    lv_style_set_text_font(&s_style_button_primary, ui_font_heading());
    lv_style_set_pad_top(&s_style_button_primary, 16);
    lv_style_set_pad_bottom(&s_style_button_primary, 16);
    lv_style_set_pad_left(&s_style_button_primary, 24);
    lv_style_set_pad_right(&s_style_button_primary, 24);
    lv_style_set_shadow_width(&s_style_button_primary, 14);
    lv_style_set_shadow_opa(&s_style_button_primary, LV_OPA_40);
    lv_style_set_shadow_color(&s_style_button_primary, ui_color_accent_glow());
    lv_style_set_shadow_ofs_y(&s_style_button_primary, 6);
    lv_style_set_transition(&s_style_button_primary, &s_button_transition);

    lv_style_init(&s_style_button_primary_pressed);
    lv_style_set_bg_color(&s_style_button_primary_pressed, lv_color_hex(0xE0631A));
    lv_style_set_bg_grad_color(&s_style_button_primary_pressed, ui_color_accent());
    lv_style_set_shadow_width(&s_style_button_primary_pressed, 6);
    lv_style_set_shadow_opa(&s_style_button_primary_pressed, LV_OPA_30);

    /* Bouton secondaire : fond doux, texte orange foncé, bord. */
    lv_style_init(&s_style_button_secondary);
    lv_style_set_bg_color(&s_style_button_secondary, ui_color_accent_soft());
    lv_style_set_bg_opa(&s_style_button_secondary, LV_OPA_COVER);
    lv_style_set_radius(&s_style_button_secondary, 26);
    lv_style_set_border_width(&s_style_button_secondary, 1);
    lv_style_set_border_color(&s_style_button_secondary, lv_color_hex(0xFFCFA8));
    lv_style_set_text_color(&s_style_button_secondary, lv_color_hex(0xE0631A));
    lv_style_set_text_font(&s_style_button_secondary, ui_font_heading());
    lv_style_set_pad_top(&s_style_button_secondary, 16);
    lv_style_set_pad_bottom(&s_style_button_secondary, 16);
    lv_style_set_pad_left(&s_style_button_secondary, 22);
    lv_style_set_pad_right(&s_style_button_secondary, 22);
    lv_style_set_shadow_width(&s_style_button_secondary, 0);
    lv_style_set_transition(&s_style_button_secondary, &s_button_transition);

    lv_style_init(&s_style_button_secondary_pressed);
    lv_style_set_bg_color(&s_style_button_secondary_pressed, ui_color_surface_highlight());
    lv_style_set_border_color(&s_style_button_secondary_pressed, ui_color_accent());

    /* Bouton désactivé. */
    lv_style_init(&s_style_button_disabled);
    lv_style_set_bg_color(&s_style_button_disabled, ui_color_disabled());
    lv_style_set_bg_grad_color(&s_style_button_disabled, ui_color_disabled());
    lv_style_set_border_width(&s_style_button_disabled, 0);
    lv_style_set_text_color(&s_style_button_disabled, ui_color_text_muted());
    lv_style_set_shadow_width(&s_style_button_disabled, 0);

    /* Titre d'écran : police display festive. */
    lv_style_init(&s_style_title);
    lv_style_set_text_color(&s_style_title, ui_color_text_primary());
    lv_style_set_text_font(&s_style_title, ui_font_display());

    /* Heading / libellé fort. */
    lv_style_init(&s_style_heading);
    lv_style_set_text_color(&s_style_heading, ui_color_text_primary());
    lv_style_set_text_font(&s_style_heading, ui_font_title());

    lv_style_init(&s_style_body);
    lv_style_set_text_color(&s_style_body, ui_color_text_secondary());
    lv_style_set_text_font(&s_style_body, ui_font_body());
    lv_style_set_text_line_space(&s_style_body, 3);

    lv_style_init(&s_style_caption);
    lv_style_set_text_color(&s_style_caption, ui_color_text_muted());
    lv_style_set_text_font(&s_style_caption, ui_font_caption());

    /* Overline : caption + tracking + couleur orange foncé. */
    lv_style_init(&s_style_overline);
    lv_style_set_text_color(&s_style_overline, lv_color_hex(0xE0631A));
    lv_style_set_text_font(&s_style_overline, ui_font_caption());
    lv_style_set_text_letter_space(&s_style_overline, 2);

    /* Bannière succès. */
    lv_style_init(&s_style_banner);
    lv_style_set_bg_color(&s_style_banner, lv_color_hex(0xE6F6EA));
    lv_style_set_bg_opa(&s_style_banner, LV_OPA_COVER);
    lv_style_set_radius(&s_style_banner, 14);
    lv_style_set_pad_all(&s_style_banner, 14);
    lv_style_set_text_color(&s_style_banner, lv_color_hex(0x1E7A38));
    lv_style_set_text_font(&s_style_banner, ui_font_body());
    lv_style_set_border_width(&s_style_banner, 1);
    lv_style_set_border_color(&s_style_banner, lv_color_hex(0xB6E2C2));

    /* Bannière alerte. */
    lv_style_init(&s_style_banner_warning);
    lv_style_set_bg_color(&s_style_banner_warning, lv_color_hex(0xFFF3D6));
    lv_style_set_bg_opa(&s_style_banner_warning, LV_OPA_COVER);
    lv_style_set_radius(&s_style_banner_warning, 14);
    lv_style_set_pad_all(&s_style_banner_warning, 14);
    lv_style_set_text_color(&s_style_banner_warning, lv_color_hex(0x9A6500));
    lv_style_set_text_font(&s_style_banner_warning, ui_font_body());
    lv_style_set_border_width(&s_style_banner_warning, 1);
    lv_style_set_border_color(&s_style_banner_warning, lv_color_hex(0xF5D58A));

    /* Bannière erreur / sécurité. */
    lv_style_init(&s_style_banner_error);
    lv_style_set_bg_color(&s_style_banner_error, lv_color_hex(0xFCE4E4));
    lv_style_set_bg_opa(&s_style_banner_error, LV_OPA_COVER);
    lv_style_set_radius(&s_style_banner_error, 14);
    lv_style_set_pad_all(&s_style_banner_error, 14);
    lv_style_set_text_color(&s_style_banner_error, lv_color_hex(0xB01217));
    lv_style_set_text_font(&s_style_banner_error, ui_font_body());
    lv_style_set_border_width(&s_style_banner_error, 1);
    lv_style_set_border_color(&s_style_banner_error, lv_color_hex(0xF3B6B8));

    /* Pastille neutre. */
    lv_style_init(&s_style_badge);
    lv_style_set_bg_color(&s_style_badge, ui_color_accent_soft());
    lv_style_set_bg_opa(&s_style_badge, LV_OPA_COVER);
    lv_style_set_radius(&s_style_badge, 20);
    lv_style_set_pad_left(&s_style_badge, 11);
    lv_style_set_pad_right(&s_style_badge, 11);
    lv_style_set_pad_top(&s_style_badge, 5);
    lv_style_set_pad_bottom(&s_style_badge, 5);
    lv_style_set_text_color(&s_style_badge, lv_color_hex(0xE0631A));
    lv_style_set_text_font(&s_style_badge, ui_font_caption());
    lv_style_set_border_width(&s_style_badge, 0);

    /* Pastille alerte. */
    lv_style_init(&s_style_badge_alert);
    lv_style_set_bg_color(&s_style_badge_alert, lv_color_hex(0xFFF3D6));
    lv_style_set_bg_opa(&s_style_badge_alert, LV_OPA_COVER);
    lv_style_set_radius(&s_style_badge_alert, 20);
    lv_style_set_pad_left(&s_style_badge_alert, 11);
    lv_style_set_pad_right(&s_style_badge_alert, 11);
    lv_style_set_pad_top(&s_style_badge_alert, 5);
    lv_style_set_pad_bottom(&s_style_badge_alert, 5);
    lv_style_set_text_color(&s_style_badge_alert, lv_color_hex(0x9A6500));
    lv_style_set_text_font(&s_style_badge_alert, ui_font_caption());
    lv_style_set_border_width(&s_style_badge_alert, 1);
    lv_style_set_border_color(&s_style_badge_alert, lv_color_hex(0xF5D58A));

    s_styles_ready = true;
}

lv_style_t *ui_style_screen(void)                   { return &s_style_screen; }
lv_style_t *ui_style_card(void)                     { return &s_style_card; }
lv_style_t *ui_style_card_highlight(void)           { return &s_style_card_highlight; }
lv_style_t *ui_style_card_inset(void)               { return &s_style_card_inset; }
lv_style_t *ui_style_button_primary(void)           { return &s_style_button_primary; }
lv_style_t *ui_style_button_primary_pressed(void)   { return &s_style_button_primary_pressed; }
lv_style_t *ui_style_button_secondary(void)         { return &s_style_button_secondary; }
lv_style_t *ui_style_button_secondary_pressed(void) { return &s_style_button_secondary_pressed; }
lv_style_t *ui_style_button_disabled(void)          { return &s_style_button_disabled; }
lv_style_t *ui_style_title(void)                    { return &s_style_title; }
lv_style_t *ui_style_heading(void)                  { return &s_style_heading; }
lv_style_t *ui_style_body(void)                     { return &s_style_body; }
lv_style_t *ui_style_caption(void)                  { return &s_style_caption; }
lv_style_t *ui_style_overline(void)                 { return &s_style_overline; }
lv_style_t *ui_style_banner(void)                   { return &s_style_banner; }
lv_style_t *ui_style_banner_warning(void)           { return &s_style_banner_warning; }
lv_style_t *ui_style_banner_error(void)             { return &s_style_banner_error; }
lv_style_t *ui_style_badge(void)                    { return &s_style_badge; }
lv_style_t *ui_style_badge_alert(void)              { return &s_style_badge_alert; }
