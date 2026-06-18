#include "ui_palette.h"

/* Direction « Blanc Fruité » — fond clair, marque orange, couleurs festives. */

lv_color_t ui_color_background(void)       { return lv_color_hex(0xFFFCF8); } /* blanc chaud */
lv_color_t ui_color_background_alt(void)   { return lv_color_hex(0xFFF7F0); }
lv_color_t ui_color_surface(void)          { return lv_color_hex(0xFFFFFF); } /* cartes */
lv_color_t ui_color_surface_alt(void)      { return lv_color_hex(0xFFF7F0); }
lv_color_t ui_color_surface_highlight(void){ return lv_color_hex(0xFFF1E6); }
lv_color_t ui_color_surface_overlay(void)  { return lv_color_hex(0xFBEFE4); }

lv_color_t ui_color_accent(void)           { return lv_color_hex(0xFF8A3D); } /* orange marque */
lv_color_t ui_color_accent_soft(void)      { return lv_color_hex(0xFFF1E6); } /* fond doux */
lv_color_t ui_color_accent_secondary(void) { return lv_color_hex(0xFFB23D); } /* fin de dégradé */
lv_color_t ui_color_accent_glow(void)      { return lv_color_hex(0xFFD2A6); }

lv_color_t ui_color_text_primary(void)     { return lv_color_hex(0x2B1B12); }
lv_color_t ui_color_text_secondary(void)   { return lv_color_hex(0x6B5648); }
lv_color_t ui_color_text_muted(void)       { return lv_color_hex(0xA2917F); }
lv_color_t ui_color_text_ink(void)         { return lv_color_hex(0xFFFFFF); } /* texte sur orange */

lv_color_t ui_color_success(void)          { return lv_color_hex(0x2E9E4F); }
lv_color_t ui_color_warning(void)          { return lv_color_hex(0xF5A623); }
lv_color_t ui_color_error(void)            { return lv_color_hex(0xE5484D); }
lv_color_t ui_color_info(void)             { return lv_color_hex(0x3D7BFF); }
lv_color_t ui_color_disabled(void)         { return lv_color_hex(0xE7DDD3); }

lv_color_t ui_color_line(void)             { return lv_color_hex(0xF0E6DC); }
lv_color_t ui_color_line_soft(void)        { return lv_color_hex(0xF6EFE8); }

lv_color_t ui_color_family(size_t index)
{
    static const uint32_t families[] = {
        0xFF8A3D, /* Agrume */
        0xF0356B, /* Fruit rouge */
        0x7BC043, /* Gingembre / herbe */
        0x9B5CD6, /* Tonic / botanique */
        0x16C2B3, /* Tropical */
        0xFFC93C, /* Citron / miel */
    };
    return lv_color_hex(families[index % (sizeof(families) / sizeof(families[0]))]);
}
