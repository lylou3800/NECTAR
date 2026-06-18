#!/usr/bin/env bash
set -euo pipefail
TTF_DIR="${1:-./ttf}"
OUT="components/nectar_ui/src/theme/fonts"
mkdir -p "$OUT"

gen() { # nom_fichier  fichier_ttf  taille
  lv_font_conv --font "$TTF_DIR/$2" --size "$3" --bpp 4 --format lvgl --no-compress \
    -r 0x20-0x7F -r 0xA0-0xFF -r 0x152-0x153 --symbols $'‘…' \
    -o "$OUT/$1.c" --force-fast-kern-format
}

gen nectar_display_30  "Baloo2-ExtraBold.ttf"     30
gen nectar_display_24  "Baloo2-ExtraBold.ttf"     24
gen nectar_hero_40     "Montserrat-ExtraBold.ttf" 40
gen nectar_bold_18     "Montserrat-ExtraBold.ttf" 18
gen nectar_semibold_22 "Montserrat-SemiBold.ttf"  22
gen nectar_text_17     "Montserrat-Regular.ttf"   17
gen nectar_text_14     "Montserrat-Regular.ttf"   14
echo "Polices générées dans $OUT"
