# Plan d'implémentation — NECTAR UI · Sous-projet 1 : Fondations visuelles

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rendre l'UI NECTAR belle, festive, française et correctement rendue (couleurs chaudes) sur le matériel, sans toucher au modèle fonctionnel.

**Architecture :** Le firmware sépare `nectar_board` (HAL + port LVGL), `nectar_app` (modèles + machine à états, services simulés) et `nectar_ui` (écrans + thème + composants). **Levier central :** tous les écrans consomment les fonctions de thème centralisées (`ui_color_*`, `ui_style_*`, `ui_font_*`). Réécrire la palette, les polices et les styles re-skinne donc automatiquement les 11 écrans ; on traite ensuite manuellement ce qui ne se propage pas (chaînes françaises, retrait d'infos techniques, débordements, feedback verre, refonte de l'accueil).

**Tech Stack :** ESP-IDF, LVGL 8.x (API `lv_disp_drv`), C11, panneau RGB 800×480, tactile GT911, PSRAM. Génération de polices via `lv_font_conv`.

## Vérification (spécificité embarqué)

Ce firmware n'a pas de harnais de tests unitaires et le travail est visuel/matériel. La vérification de chaque tâche est donc : **(a)** `idf.py build` réussit, **(b)** flash + **observation à l'écran** selon les attendus explicites de la tâche, **(c)** commit. Le « test qui échoue d'abord » du TDD classique est remplacé, là où c'est pertinent, par une **mire/observation de référence** (ex. mire de couleurs pour le correctif R↔B). C'est l'équivalent honnête du cycle rouge→vert dans ce domaine.

Commandes de build/flash (depuis `firmware/`) :
```bash
idf.py build           # compiler
idf.py flash monitor   # flasher + console série
```

## Global Constraints

- Résolution fixe **800×480** (`UI_SCREEN_WIDTH`/`UI_SCREEN_HEIGHT`, `BOARD_DISPLAY_H_RES/V_RES`).
- **100 % des chaînes visibles en français, accents rendus correctement.** Ton : tutoiement chaleureux/festif.
- **Aucune info technique sur les écrans publics** : pas de « ±5 mL », « dosage précis », « détection de verre active/automatique », noms d'étapes internes (« Dose check »…), « stock % » / télémétrie. Le détail technique n'existe **que** sur les écrans Service/Admin (`screen_admin_auth`, `screen_maintenance`).
- **Aucune couleur en dur dans les écrans** : passer par `ui_color_*()`.
- **Préserver la fluidité** : double framebuffer PSRAM + flush synchronisé vsync + `direct_mode`/`full_refresh` (dans `lvgl_port.c`) — ne rien introduire qui provoque flicker/tearing. Pas d'animation dans ce sous-projet (réservé SP4).
- Direction visuelle « Blanc Fruité » : fond `#FFFCF8`, marque orange `#FF8A3D`, texte `#2B1B12`, couleurs par famille de cocktail, états OK/alerte/épuisé.
- LVGL 8.x : conserver l'API existante (`lv_disp_drv`, `lv_btn_create`, `lv_obj_add_style`…).

---

## Task 1 : Corriger le rendu couleur (R↔B)

**But :** diagnostiquer puis corriger l'inversion Rouge↔Bleu pour que les couleurs chaudes s'affichent chaudes. Prérequis absolu de toute la suite.

**Files:**
- Modify (temporaire) : `components/nectar_ui/src/screens/screen_boot.c`
- Modify : `components/nectar_board/src/board_display.c:182-199` (tableau `data_gpio_nums`)

**Interfaces:**
- Consumes : néant.
- Produces : néant (correction matérielle ; aucun symbole nouveau).

- [ ] **Step 1 : Ajouter une mire de couleurs temporaire dans le boot**

Dans `screen_boot_create`, **tout au début** (juste après l'ouverture de la fonction, avant `lv_obj_t *content = ...`), insérer 3 bandes pleines rouge/vert/bleu pour diagnostic :

```c
    /* --- MIRE DIAGNOSTIC COULEUR (temporaire, retirée au Step 6) --- */
    lv_obj_t *mire_r = lv_obj_create(screen);
    lv_obj_remove_style_all(mire_r);
    lv_obj_set_size(mire_r, 800, 60);
    lv_obj_align(mire_r, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(mire_r, lv_color_hex(0xFF0000), 0); /* doit être ROUGE */
    lv_obj_set_style_bg_opa(mire_r, LV_OPA_COVER, 0);

    lv_obj_t *mire_g = lv_obj_create(screen);
    lv_obj_remove_style_all(mire_g);
    lv_obj_set_size(mire_g, 800, 60);
    lv_obj_align(mire_g, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_style_bg_color(mire_g, lv_color_hex(0x00FF00), 0); /* doit être VERT */
    lv_obj_set_style_bg_opa(mire_g, LV_OPA_COVER, 0);

    lv_obj_t *mire_b = lv_obj_create(screen);
    lv_obj_remove_style_all(mire_b);
    lv_obj_set_size(mire_b, 800, 60);
    lv_obj_align(mire_b, LV_ALIGN_TOP_MID, 0, 120);
    lv_obj_set_style_bg_color(mire_b, lv_color_hex(0x0000FF), 0); /* doit être BLEU */
    lv_obj_set_style_bg_opa(mire_b, LV_OPA_COVER, 0);
    /* --- FIN MIRE --- */
```

- [ ] **Step 2 : Build + flash, observer la mire**

Run : `idf.py build flash monitor`
Attendu du diagnostic : noter l'ordre réel affiché des 3 bandes.
- Si bande 1 = BLEU et bande 3 = ROUGE (vert au milieu OK) → **inversion R↔B confirmée** → Step 3.
- Si les 3 bandes sont déjà correctes (rouge/vert/bleu) → pas d'inversion : **ne pas** faire le Step 3, documenter et passer au Step 6 (le souci serait ailleurs ; rouvrir le diagnostic avec l'utilisateur).

- [ ] **Step 3 : Permuter les groupes GPIO R0–R4 et B0–B4**

Dans `components/nectar_board/src/board_display.c`, remplacer le tableau `data_gpio_nums` actuel :

```c
        .data_gpio_nums = {
            GPIO_NUM_14,
            GPIO_NUM_38,
            GPIO_NUM_18,
            GPIO_NUM_17,
            GPIO_NUM_10,
            GPIO_NUM_39,
            GPIO_NUM_0,
            GPIO_NUM_45,
            GPIO_NUM_48,
            GPIO_NUM_47,
            GPIO_NUM_21,
            GPIO_NUM_1,
            GPIO_NUM_2,
            GPIO_NUM_42,
            GPIO_NUM_41,
            GPIO_NUM_40,
        },
```

par (les 5 premiers — groupe B — et les 5 derniers — groupe R — sont échangés ; le groupe vert central est inchangé) :

```c
        .data_gpio_nums = {
            /* B0..B4 reçoivent les anciennes lignes R0..R4 */
            GPIO_NUM_1,
            GPIO_NUM_2,
            GPIO_NUM_42,
            GPIO_NUM_41,
            GPIO_NUM_40,
            /* G0..G5 inchangés */
            GPIO_NUM_39,
            GPIO_NUM_0,
            GPIO_NUM_45,
            GPIO_NUM_48,
            GPIO_NUM_47,
            GPIO_NUM_21,
            /* R0..R4 reçoivent les anciennes lignes B0..B4 */
            GPIO_NUM_14,
            GPIO_NUM_38,
            GPIO_NUM_18,
            GPIO_NUM_17,
            GPIO_NUM_10,
        },
```

- [ ] **Step 4 : Build + flash, re-observer la mire**

Run : `idf.py build flash monitor`
Attendu : bande 1 = **ROUGE**, bande 2 = **VERT**, bande 3 = **BLEU**. L'inversion est corrigée.

- [ ] **Step 5 : Vérifier sur les écrans existants**

Toujours flashé, laisser le boot finir : les accents dorés/crème de l'ancien thème doivent maintenant apparaître **chauds** (or/ambre), plus aucun cyan. (Le thème sera remplacé ensuite ; ici on valide juste le rendu.)

- [ ] **Step 6 : Retirer la mire temporaire**

Supprimer intégralement le bloc inséré au Step 1 dans `screen_boot.c`. Rebuild pour confirmer.
Run : `idf.py build`
Attendu : build OK, boot screen sans bandes.

- [ ] **Step 7 : Commit**

```bash
git add components/nectar_board/src/board_display.c components/nectar_ui/src/screens/screen_boot.c
git commit -m "fix(board): corriger l'inversion R<->B du panneau RGB (ordre des lignes de donnees)"
```

---

## Task 2 : Palette claire « Blanc Fruité »

**But :** remplacer les valeurs sombres par la palette claire et ajouter les accesseurs « famille de cocktail » et `info`. Re-skinne automatiquement les couleurs de tous les écrans.

**Files:**
- Modify : `components/nectar_ui/include/ui_palette.h`
- Modify : `components/nectar_ui/src/theme/ui_palette.c`

**Interfaces:**
- Produces :
  - Fonctions existantes conservées (mêmes signatures `lv_color_t ui_color_*(void)`), valeurs claires.
  - Nouveau : `lv_color_t ui_color_info(void);`
  - Nouveau : `lv_color_t ui_color_family(size_t index);` (cycle sur 6 couleurs de famille, déterministe par index de recette).

- [ ] **Step 1 : Mettre à jour l'en-tête**

Dans `components/nectar_ui/include/ui_palette.h`, ajouter `#include <stddef.h>` après `#include "lvgl.h"`, puis ajouter ces deux déclarations avant `#ifdef __cplusplus` de fermeture :

```c
lv_color_t ui_color_info(void);
lv_color_t ui_color_family(size_t index);
```

- [ ] **Step 2 : Réécrire `ui_palette.c`**

Remplacer **tout** le contenu de `components/nectar_ui/src/theme/ui_palette.c` par :

```c
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
```

- [ ] **Step 3 : Build**

Run : `idf.py build`
Attendu : compilation OK (aucun appelant cassé : signatures conservées, 2 ajouts).

- [ ] **Step 4 : Flash + observer**

Run : `idf.py flash monitor`
Attendu : les écrans existants passent en **fond clair**, texte brun foncé, accents orange. La structure reste l'ancienne (sera retravaillée), mais l'ambiance est claire/chaude.

- [ ] **Step 5 : Commit**

```bash
git add components/nectar_ui/include/ui_palette.h components/nectar_ui/src/theme/ui_palette.c
git commit -m "feat(ui): palette claire Blanc Fruite + couleurs par famille"
```

---

## Task 3 : Polices françaises + typographie

**But :** générer des polices incluant les accents FR (Montserrat + display Baloo 2), les enregistrer, et réécrire les accesseurs typo avec une échelle agrandie.

**Files:**
- Create : `tools/gen_fonts.sh`
- Create : `components/nectar_ui/src/theme/fonts/*.c` (générés)
- Create : `components/nectar_ui/src/theme/fonts/nectar_fonts.h`
- Modify : `components/nectar_ui/include/ui_typography.h`
- Modify : `components/nectar_ui/src/theme/ui_typography.c`

**Interfaces:**
- Produces (accesseurs typo) :
  - `const lv_font_t *ui_font_display(void);`  → Baloo 2 ExtraBold 30 (titres d'écran festifs)
  - `const lv_font_t *ui_font_wordmark(void);` → Baloo 2 ExtraBold 24 (wordmark NECTAR) **[nouveau]**
  - `const lv_font_t *ui_font_hero(void);`     → Montserrat ExtraBold 40 (grands nombres) **[nouveau]**
  - `const lv_font_t *ui_font_title(void);`    → Montserrat SemiBold 22 (titres de section / noms)
  - `const lv_font_t *ui_font_heading(void);`  → Montserrat ExtraBold 18 (boutons, libellés forts)
  - `const lv_font_t *ui_font_body(void);`     → Montserrat Regular 17 (corps)
  - `const lv_font_t *ui_font_caption(void);`  → Montserrat Regular 14 (légendes / overline)
- Polices générées (noms de variables) : `nectar_display_30`, `nectar_display_24`, `nectar_hero_40`, `nectar_bold_18`, `nectar_semibold_22`, `nectar_text_17`, `nectar_text_14`.

- [ ] **Step 1 : Récupérer les fichiers TTF**

Télécharger dans un dossier de travail (hors dépôt) :
- Montserrat (Regular, SemiBold, ExtraBold) — https://fonts.google.com/specimen/Montserrat
- Baloo 2 (ExtraBold) — https://fonts.google.com/specimen/Baloo+2

- [ ] **Step 2 : Installer le convertisseur**

Run : `npm install -g lv_font_conv`
(Alternative sans Node : convertisseur en ligne LVGL https://lvgl.io/tools/fontconverter avec les mêmes plages.)

- [ ] **Step 3 : Créer le script de génération**

Créer `tools/gen_fonts.sh` (adapter `TTF_DIR` au dossier des `.ttf`). La plage couvre ASCII + Latin-1 (accents FR) + Œœ + apostrophe typographique + points de suspension :

```bash
#!/usr/bin/env bash
set -euo pipefail
TTF_DIR="${1:-./ttf}"
OUT="components/nectar_ui/src/theme/fonts"
RANGE="-r 0x20-0x7F -r 0xA0-0xFF -r 0x152-0x153 --symbols '’…'"
mkdir -p "$OUT"

gen() { # nom_fichier  fichier_ttf  taille
  lv_font_conv --font "$TTF_DIR/$2" --size "$3" --bpp 4 --format lvgl --no-compress \
    -r 0x20-0x7F -r 0xA0-0xFF -r 0x152-0x153 --symbols '’…' \
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
```

- [ ] **Step 4 : Générer les polices**

Run : `bash tools/gen_fonts.sh /chemin/vers/ttf`
Attendu : 7 fichiers `.c` créés dans `components/nectar_ui/src/theme/fonts/`. Chaque fichier définit `lv_font_t <nom>` (ex. `nectar_display_30`).

- [ ] **Step 5 : Créer l'en-tête de déclaration des polices**

Créer `components/nectar_ui/src/theme/fonts/nectar_fonts.h` :

```c
#pragma once

#include "lvgl.h"

LV_FONT_DECLARE(nectar_display_30);
LV_FONT_DECLARE(nectar_display_24);
LV_FONT_DECLARE(nectar_hero_40);
LV_FONT_DECLARE(nectar_bold_18);
LV_FONT_DECLARE(nectar_semibold_22);
LV_FONT_DECLARE(nectar_text_17);
LV_FONT_DECLARE(nectar_text_14);
```

- [ ] **Step 6 : Mettre à jour `ui_typography.h`**

Remplacer le bloc de déclarations de `components/nectar_ui/include/ui_typography.h` par :

```c
const lv_font_t *ui_font_display(void);
const lv_font_t *ui_font_wordmark(void);
const lv_font_t *ui_font_hero(void);
const lv_font_t *ui_font_title(void);
const lv_font_t *ui_font_heading(void);
const lv_font_t *ui_font_body(void);
const lv_font_t *ui_font_caption(void);
```

- [ ] **Step 7 : Réécrire `ui_typography.c`**

Remplacer **tout** le contenu par :

```c
#include "ui_typography.h"

#include "fonts/nectar_fonts.h"

const lv_font_t *ui_font_display(void)  { return &nectar_display_30; }
const lv_font_t *ui_font_wordmark(void) { return &nectar_display_24; }
const lv_font_t *ui_font_hero(void)     { return &nectar_hero_40; }
const lv_font_t *ui_font_title(void)    { return &nectar_semibold_22; }
const lv_font_t *ui_font_heading(void)  { return &nectar_bold_18; }
const lv_font_t *ui_font_body(void)     { return &nectar_text_17; }
const lv_font_t *ui_font_caption(void)  { return &nectar_text_14; }
```

- [ ] **Step 8 : Reconfigurer (re-glob CMake) + build**

Run : `idf.py reconfigure && idf.py build`
Attendu : les nouveaux `.c` de polices sont pris par le `GLOB_RECURSE`, build OK.

- [ ] **Step 9 : Flash + observer les accents**

Run : `idf.py flash monitor`
Attendu : texte plus grand ; pour valider les accents, vérifier visuellement après la Task 6 (chaînes FR). Ici, au minimum, le wordmark « NECTAR » du boot est plus grand et rond.

- [ ] **Step 10 : Commit**

```bash
git add tools/gen_fonts.sh components/nectar_ui/src/theme/fonts components/nectar_ui/include/ui_typography.h components/nectar_ui/src/theme/ui_typography.c
git commit -m "feat(ui): polices FR (Montserrat + Baloo) et echelle typo agrandie"
```

---

## Task 4 : Styles clairs (cartes, boutons, bannières, pastilles)

**But :** réécrire `ui_styles.c` pour le thème clair, en mappant les polices aux bons rôles.

**Files:**
- Modify : `components/nectar_ui/src/theme/ui_styles.c`

**Interfaces:**
- Consumes : `ui_color_*()` (Task 2), `ui_font_*()` (Task 3).
- Produces : mêmes accesseurs `ui_style_*()` (signatures inchangées) avec rendu clair.

- [ ] **Step 1 : Réécrire `ui_styles.c`**

Remplacer **tout** le contenu de `components/nectar_ui/src/theme/ui_styles.c` par le code ci-dessous (mêmes styles exposés, valeurs claires ; titres en police display, ombres douces sur fond clair) :

```c
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
```

- [ ] **Step 2 : Build + flash + observer**

Run : `idf.py build flash monitor`
Attendu : cartes blanches à ombre douce, boutons orange dégradé/texte blanc, pastilles claires, titres en police ronde. Les écrans ressemblent aux maquettes (hors textes encore anglais et débordements résiduels, traités ensuite).

- [ ] **Step 3 : Commit**

```bash
git add components/nectar_ui/src/theme/ui_styles.c
git commit -m "feat(ui): styles clairs (cartes, boutons, bannieres, pastilles)"
```

---

## Task 5 : Composants — couleur par famille, retrait des artefacts sombres

**But :** adapter les composants qui dessinent des couleurs « en dur » sombres (halos/anneaux pensés pour fond noir) au thème clair, et colorer la carte cocktail selon sa famille. Index de famille = index de carte (déterministe, sans toucher au modèle).

**Files:**
- Modify : `components/nectar_ui/src/components/ui_card_recipe.c`
- Modify : `components/nectar_ui/src/components/ui_glass_guide.c`
- Modify : `components/nectar_ui/src/components/ui_progress_panel.c`
- Modify : `components/nectar_ui/include/ui_card_recipe.h`
- Modify : `components/nectar_ui/src/screens/screen_recipes.c:303` (passer l'index famille)

**Interfaces:**
- Consumes : `ui_color_family(size_t)` (Task 2), `ui_font_hero()` (Task 3).
- Produces : signature carte enrichie d'un paramètre famille :
  `lv_obj_t *ui_card_recipe_create(lv_obj_t *parent, const recipe_model_t *recipe, size_t family_index, lv_event_cb_t event_cb, void *user_data);`

- [ ] **Step 1 : Mettre à jour l'en-tête de la carte**

Dans `components/nectar_ui/include/ui_card_recipe.h`, remplacer la déclaration de `ui_card_recipe_create` par (ajout de `size_t family_index` ; ajouter `#include <stddef.h>` si absent) :

```c
lv_obj_t *ui_card_recipe_create(lv_obj_t *parent,
                                const recipe_model_t *recipe,
                                size_t family_index,
                                lv_event_cb_t event_cb,
                                void *user_data);
```

- [ ] **Step 2 : Colorer la carte selon la famille**

Dans `ui_card_recipe.c`, modifier la signature de la fonction pour ajouter `size_t family_index` (même ordre que l'en-tête). Puis, dans le bloc `else` du halo (cas sans `visual_src`, lignes ~137-156), remplacer la couleur du halo et du monogramme par la couleur de famille :

Remplacer :
```c
        lv_obj_set_style_bg_color(halo, recipe->available ? ui_color_accent() : ui_color_disabled(), 0);
        lv_obj_set_style_bg_opa(halo, recipe->available ? LV_OPA_30 : LV_OPA_20, 0);
        lv_obj_set_style_shadow_width(halo, 8, 0);
        lv_obj_set_style_shadow_color(halo, recipe->available ? ui_color_accent_glow() : ui_color_surface_overlay(), 0);
        lv_obj_set_style_shadow_opa(halo, recipe->available ? LV_OPA_20 : LV_OPA_10, 0);
```
par :
```c
        lv_obj_set_style_bg_color(halo, recipe->available ? ui_color_family(family_index) : ui_color_disabled(), 0);
        lv_obj_set_style_bg_opa(halo, recipe->available ? LV_OPA_COVER : LV_OPA_40, 0);
        lv_obj_set_style_shadow_width(halo, 10, 0);
        lv_obj_set_style_shadow_color(halo, recipe->available ? ui_color_family(family_index) : ui_color_disabled(), 0);
        lv_obj_set_style_shadow_opa(halo, recipe->available ? LV_OPA_30 : LV_OPA_10, 0);
```

Et pour le monogramme (ligne ~153), forcer texte blanc lisible sur le halo coloré :
Remplacer :
```c
        lv_obj_set_style_text_color(monogram, recipe->available ? ui_color_text_primary() : ui_color_text_muted(), 0);
```
par :
```c
        lv_obj_set_style_text_color(monogram, recipe->available ? ui_color_text_ink() : ui_color_text_muted(), 0);
```

- [ ] **Step 3 : Passer l'index famille à la création des cartes**

Dans `components/nectar_ui/src/screens/screen_recipes.c`, à l'appel `ui_card_recipe_create` (ligne ~303), remplacer :
```c
        ui_card_recipe_create(s_recipe_carousel, recipe, recipe_select_cb, (void *)(uintptr_t)index);
```
par :
```c
        ui_card_recipe_create(s_recipe_carousel, recipe, index, recipe_select_cb, (void *)(uintptr_t)index);
```

- [ ] **Step 4 : Adapter le glass guide au fond clair**

Dans `ui_glass_guide.c`, l'anneau et le halo utilisent déjà `ui_color_*`. Renforcer le contraste sur fond clair : dans `ui_glass_guide_create`, après la création de `glow`, changer son opacité de fond de `LV_OPA_10` à `LV_OPA_COVER` et sa couleur reste `ui_color_accent()` :
Remplacer :
```c
    lv_obj_set_style_bg_color(glow, ui_color_accent(), 0);
    lv_obj_set_style_bg_opa(glow, LV_OPA_10, 0);
```
par :
```c
    lv_obj_set_style_bg_color(glow, ui_color_accent(), 0);
    lv_obj_set_style_bg_opa(glow, LV_OPA_20, 0);
```
(Les textes anglais de ce composant sont francisés en Task 6 / Task 7.)

- [ ] **Step 5 : Grand nombre du panneau de progression en police hero**

Dans `ui_progress_panel.c`, le `percent` utilise `ui_font_display()`. Le passer en `ui_font_hero()` et couleur accent foncé lisible :
Remplacer :
```c
    lv_obj_set_style_text_font(percent, ui_font_display(), 0);
    lv_obj_set_style_text_color(percent, ui_color_accent_secondary(), 0);
```
par :
```c
    lv_obj_set_style_text_font(percent, ui_font_hero(), 0);
    lv_obj_set_style_text_color(percent, ui_color_accent(), 0);
```

- [ ] **Step 6 : Build + flash + observer**

Run : `idf.py build flash monitor`
Attendu : cartes du menu avec pastille colorée par famille (orange / rose / vert…), monogramme blanc lisible, anneau de verre net sur fond clair, grand % de progression en gros chiffres.

- [ ] **Step 7 : Commit**

```bash
git add components/nectar_ui/src/components components/nectar_ui/include/ui_card_recipe.h components/nectar_ui/src/screens/screen_recipes.c
git commit -m "feat(ui): couleur par famille sur les cartes + composants adaptes au fond clair"
```

---

## Task 6 : Francisation + retrait des infos techniques (écrans & composants)

**But :** traduire toutes les chaînes visibles en français et retirer les infos techniques des écrans publics, en appliquant les règles de contenu. Remplacements exacts ci-dessous (anglais → français).

**Files:** tous les `screen_*.c`, les composants à textes, et les données `recipe_model.c` / `allergen_model.h`.

**Interfaces:** aucune signature modifiée (remplacements de chaînes uniquement, sauf retraits de champs d'affichage indiqués).

- [ ] **Step 1 : Boot** — `components/nectar_ui/src/screens/screen_boot.c`
  - `"DARK COCKTAIL EXPERIENCE"` → `"LE BAR FESTIF"`
  - `"Initializing the touch service, crafting engine and safety flow."` → `"Préparation du service et de l'expérience tactile…"`
  - `"Preparing the 7-inch display and guided serving journey."` → `"On allume le bar, c'est presque prêt."`
  - Passer le wordmark `brand` en police wordmark : remplacer `lv_obj_set_style_text_font(brand, ui_font_display(), 0);` par `lv_obj_set_style_text_font(brand, ui_font_wordmark(), 0);`
  - `loading` (le « 74% ») : remplacer `lv_obj_set_style_text_font(loading, ui_font_display(), 0);` par `lv_obj_set_style_text_font(loading, ui_font_hero(), 0);`

- [ ] **Step 2 : Recipes** — `components/nectar_ui/src/screens/screen_recipes.c`
  - En-tête : `"CELLAR MENU"` → `"LE MENU"` ; `"Signature cocktails"` → `"Choisis ton cocktail"` ; `"Slide through the selection and open a card for full recipe details."` → `"Fais défiler et touche une carte pour la découvrir."`
  - `recipes_status_text` : `"Unavailable"` → `"Indisponible"` ; `"Low stock"` → `"Bientôt épuisé"` ; `"Ready to pour"` → `"Prêt à servir"`.
  - `"CELLAR CURATION"` → `"SÉLECTION"`.
  - Meta (retrait du stock %, info technique) : remplacer le bloc `lv_label_set_text_fmt(s_recipe_meta_label, "%u mL serve\n%u ingredient%s  |  stock %u%%", ...)` par :
    ```c
        lv_label_set_text_fmt(
            s_recipe_meta_label,
            "%u mL  ·  %u ingrédient%s",
            recipe->total_ml,
            recipe->ingredient_count,
            recipe->ingredient_count > 1U ? "s" : ""
        );
    ```
    (supprimer l'argument `recipe->stock_percent` devenu inutilisé dans cet appel)
  - Swipe label : `"Swipe to browse %u signatures"` → `"Glisse pour voir les %u cocktails"` ; `"Tap to open this signature"` → `"Touche pour ouvrir ce cocktail"`.
  - `"Swipe sideways to browse the menu."` → `"Glisse sur le côté pour parcourir le menu."`
  - `"Ready to pour"` (badge init ligne ~260) → `"Prêt à servir"`.
  - Boutons : `"Home"` → `"Accueil"` ; `"Craft blend"` → `"Composer mon verre"`.

- [ ] **Step 3 : Drink detail** — `components/nectar_ui/src/screens/screen_drink_detail.c`
  - `"RECIPE DETAIL"` → `"LA RECETTE"` (les 2 occurrences) ; `"Recipe not found"` → `"Cocktail introuvable"` ; `"Return to the menu and choose another option."` → `"Reviens au menu et choisis-en un autre."`
  - `"CURATED SERVE"` → `"À DÉCOUVRIR"`.
  - Badge : `"LOW STOCK"` → `"BIENTÔT ÉPUISÉ"` ; `"READY"` → `"PRÊT"`.
  - `"%u mL crafted serve"` → `"%u mL par verre"`.
  - `"BLEND BREAKDOWN"` → `"COMPOSITION"`.
  - `"Measured for one glass with clear ratios, allergen visibility and a consistent finish."` → `"Composition d'un verre : ingrédients et allergènes en clair."` (retrait de toute allusion technique).
  - `"%u mL total volume"` → `"%u mL au total"`.
  - Boutons : `"Back"` → `"Retour"` ; `"Start serving"` → `"Servir"`.

- [ ] **Step 4 : Custom mix** — `components/nectar_ui/src/screens/screen_custom_mix.c`
  - En-tête : `"CUSTOM BLEND"` → `"MON MÉLANGE"` ; `"Build your own pour"` → `"Compose ton verre"` ; `"Adjust each line, then confirm the final volume before service."` → `"Règle chaque ingrédient, puis lance le service."` (retrait « ±5 mL »/précision).
  - `"LIVE MIXER"` → `"INGRÉDIENTS"`.
  - `"%u mL selected"` → `"%u mL au total"` (les 3 occurrences : `custom_mix_sync_live_labels`, init ligne ~88, `screen_custom_mix_refresh`).
  - `"0 mL selected"` → `"0 mL au total"`.
  - Boutons : `"Back"` → `"Retour"` ; `"Start serving"` → `"Servir"`.

- [ ] **Step 5 : Glass check** — `components/nectar_ui/src/screens/screen_glass_check.c` (le feedback enrichi est en Task 7 ; ici juste FR)
  - En-tête : `"STEP 1"` → `"ÉTAPE 1"` ; `"Place your glass"` → `"Pose ton verre"` ; sous-titre → `"Le service démarre dès que ton verre est bien posé."`
  - Texte détecté → `"Verre détecté, c'est parti !"`
  - Texte non détecté → `"Pose ton verre sur l'emplacement pour commencer."`

- [ ] **Step 6 : Glass guide (composant)** — `components/nectar_ui/src/components/ui_glass_guide.c`
  - `"SAFETY CHECK"` → `"EMPLACEMENT"`.
  - `"Place the glass inside the illuminated ring."` (2 occurrences : create + update) → `"Pose ton verre dans l'anneau."`
  - hint (2 occurrences) → `"Le service démarre automatiquement dès que ton verre est en place."`
  - `"WAITING FOR SENSOR"` (2 occurrences) → `"EN ATTENTE"`.
  - Détecté : `"Glass detected and locked for service."` → `"Verre détecté, c'est parti !"` ; hint détecté → `"Garde ton verre en place pendant la préparation."` ; `"SENSOR READY"` → `"PRÊT"`.

- [ ] **Step 7 : Preparing** — `components/nectar_ui/src/screens/screen_preparing.c`
  - `"STEP 2"` → `"ÉTAPE 2"` ; `"Preparing your order"` → `"On prépare ton verre"` ; sous-titre → `"Encore quelques instants…"`
  - hint → `"Garde ton verre en place, ta boisson arrive."`

- [ ] **Step 8 : Progress panel (composant)** — `components/nectar_ui/src/components/ui_progress_panel.c`
  - `"PREPARATION"` → `"PRÉPARATION"`.
  - `"Crafting your drink"` → `"On prépare ton verre…"`
  - `"GLASS LOCKED"` → `"VERRE EN PLACE"`.
  - `"The full sequence is redrawn cleanly while the machine advances through each controlled phase."` → `"Ta boisson se prépare, plus que quelques secondes."`
  - `"Initializing sequence"` → `"Démarrage…"`

- [ ] **Step 9 : Ready** — `components/nectar_ui/src/screens/screen_ready.c`
  - `"SERVICE COMPLETE"` → `"C'EST PRÊT"` ; `"Your drink is ready"` → `"Ta boisson est prête !"` ; sous-titre → `"Récupère ton verre, bonne dégustation."`
  - `"PICKUP NOW"` → `"À TOI DE JOUER"`.
  - `"Your drink is ready for pickup."` → `"Ta boisson t'attend."`
  - `"%s is complete."` → `"%s, c'est prêt !"` ; `"Your custom blend is complete."` → `"Ton mélange est prêt !"`
  - `"Lift the glass when ready. The menu will reopen automatically after the final cycle clears."` → `"Prends ton verre. Le menu revient tout seul dans un instant."`
  - Bouton : `"Back home"` → `"Accueil"`.

- [ ] **Step 10 : Error** — `components/nectar_ui/src/screens/screen_error.c`
  - `"SERVICE NOTICE"` → `"INFO"` ; `"An action is required"` → `"Petit souci"` ; sous-titre → `"Pas de panique, voici quoi faire."`
  - `"ATTENTION"` → `"ATTENTION"` (inchangé).
  - Bouton : `"Back home"` → `"Accueil"`.

- [ ] **Step 11 : Messages d'erreur du contrôleur** — `components/nectar_app/src/app_controller.c`
  - `"Recipe not found"` / `"The selected recipe does not exist."` → `"Cocktail introuvable"` / `"Ce cocktail n'existe pas."`
  - `"Recipe unavailable"` / `"A critical ingredient is empty right now. Please choose another recipe."` → `"Cocktail indisponible"` / `"Un ingrédient est épuisé. Choisis un autre cocktail."`
  - `"Selection required"` / `"Choose a recipe or create a custom mix before continuing."` → `"Fais ton choix"` / `"Choisis un cocktail ou compose ton mélange."`
  - `"Enter the 4-digit admin PIN."` → `"Entre le code à 4 chiffres."` (2 occurrences)
  - `"Tap Unlock to open the admin dashboard."` → `"Appuie sur Déverrouiller."`
  - `"PIN must contain exactly 4 digits."` → `"Le code fait 4 chiffres."`
  - `"Incorrect PIN. Try again."` → `"Code incorrect. Réessaie."`
  - `"Access granted."` → `"Accès autorisé."`
  - `prepare_step` `"Waiting for glass"` → `"En attente du verre"` ; `"Dose check"` → `"Préparation"`.

- [ ] **Step 12 : Admin auth** — `components/nectar_ui/src/screens/screen_admin_auth.c` (écran Service : on peut rester sobre)
  - `"SERVICE ACCESS"` → `"ACCÈS SERVICE"` ; `"Unlock diagnostics"` → `"Accès maintenance"` ; sous-titre → `"Entre le code pour accéder à la maintenance."`
  - `"AUTHORIZED STAFF"` → `"PERSONNEL AUTORISÉ"`.
  - `"Use the keypad to unlock service mode. Public ordering remains separate from this space."` → `"Utilise le clavier pour ouvrir le mode service."`
  - `"PIN"` → `"CODE"`.
  - Détection « Incorrect » dans `admin_auth_apply_state` : `strstr(state->admin_pin_status, "Incorrect")` → adapter à `"incorrect"` (chaîne FR « Code incorrect. Réessaie. ») : remplacer `"Incorrect"` par `"incorrect"`.
  - Touches : `"Cancel"` → `"Annuler"` ; `"Delete"` → `"Effacer"`.
  - Boutons : `"Cancel"` → `"Annuler"` ; `"Unlock"` → `"Déverrouiller"`.

- [ ] **Step 13 : Maintenance** — `components/nectar_ui/src/screens/screen_maintenance.c` (écran Service : détail technique conservé, mais en FR)
  - `admin_machine_state_text` : `"Waiting for glass"`→`"En attente du verre"` ; `"Glass detected"`→`"Verre détecté"` ; `"Preparing"`→`"Préparation"` ; `"Ready"`→`"Prêt"` ; `"Unavailable"`→`"Indisponible"` ; `"Error"`→`"Erreur"` ; `"Admin active"`→`"Mode service"` ; `"Idle"`→`"Au repos"`.
  - En-tête : `"SERVICE DECK"` → `"ESPACE SERVICE"` ; `"Maintenance overview"` → `"Niveaux & état"` ; sous-titre → `"Niveaux des réservoirs et état de la machine."`
  - Bouton `"Close service"` → `"Fermer le service"`.
  - Tuiles `"Machine"` / `"Prepare"` / `"Sensor"` → `"Machine"` / `"Préparation"` / `"Capteur verre"`.
  - `"Glass lock active"` → `"Verre en place"` ; `"Waiting for lock"` → `"En attente du verre"`.
  - États réservoir create : `"Critical refill window"`→`"Recharge critique"` ; `"Stable serving range"`→`"Niveau correct"`.
  - États refresh : `"Immediate refill needed"`→`"Recharge immédiate"` ; `"Refill window open"`→`"À recharger bientôt"` ; `"Stable serving range"`→`"Niveau correct"`.
  - Hints : `"Refill soon."`→`"À recharger."` ; `"Ready for service."`→`"OK pour le service."` ; `"Critical level (%u%%)."`→`"Niveau critique (%u%%)."` ; `"Refill soon (%u%%)."`→`"À recharger (%u%%)."` ; `"Ready (%u%%)."`→`"OK (%u%%)."`

- [ ] **Step 14 : Données recettes / allergènes**
  - `components/nectar_app/include/allergen_model.h` : `ALLERGEN_CAFFEINE "Cafeine"` → `"Caféine"`.
  - `components/nectar_app/src/recipe_model.c` taglines : `"Fresh, bright, quick serve"`→`"Frais et pétillant"` ; `"Soft spice, lively finish"`→`"Épicé et vif"` ; `"Premium fruit profile"`→`"Fruité et intense"` ; `"Dry, crisp, very clean"`→`"Sec et net"`. Ingrédients : `"Citrus soda"`→`"Soda agrumes"` ; `"Dry tonic"`→`"Tonic sec"` ; `"Ginger ale"`→`"Ginger ale"` (inchangé). (Les noms de cocktails « Citrus Spritz », « Ginger Fizz », « Ruby Sunset », « Night Tonic » restent — noms de produit.)

- [ ] **Step 15 : Build + flash + observer (accents !)**

Run : `idf.py build flash monitor`
Attendu : tous les écrans en français, **accents corrects** (é, è, à, ç…), aucune info technique sur les écrans publics, détail technique conservé côté Service.

- [ ] **Step 16 : Commit**

```bash
git add components/nectar_ui components/nectar_app
git commit -m "feat(ui): francisation complete + retrait des infos techniques des ecrans publics"
```

---

## Task 7 : Feedback « Pose ton verre » renforcé

**But :** rendre le prompt « verre absent » visuellement clair et amical (sans technique), conformément à la règle UX. (Animation = SP4 ; ici visuel statique + couleurs/icône.)

**Files:**
- Modify : `components/nectar_ui/src/screens/screen_glass_check.c`

**Interfaces:**
- Consumes : `ui_style_banner` / `ui_style_banner_warning`, `ui_color_*`.

- [ ] **Step 1 : Bannière de statut explicite selon l'état**

Dans `screen_glass_check.c`, `screen_glass_check_refresh`, remplacer le bloc `if (state->glass_detected) { ... } else { ... }` par une bannière qui change de **style** (succès vs alerte) et de **texte amical** :

```c
    ui_glass_guide_update(s_glass_guide, state->glass_detected);
    if (state->glass_detected) {
        lv_obj_remove_style(s_glass_status, ui_style_banner_warning(), 0);
        lv_obj_add_style(s_glass_status, ui_style_banner(), 0);
        lv_label_set_text(s_glass_status, LV_SYMBOL_OK "  Verre détecté, c'est parti !");
    } else {
        lv_obj_remove_style(s_glass_status, ui_style_banner(), 0);
        lv_obj_add_style(s_glass_status, ui_style_banner_warning(), 0);
        lv_label_set_text(s_glass_status, LV_SYMBOL_DOWNLOAD "  Pose ton verre sur l'emplacement pour commencer.");
    }
```

(Le style de base `ui_style_banner` reste ajouté à la création ; on superpose/retire le style alerte selon l'état. `LV_SYMBOL_*` font partie de la fonte symbole intégrée LVGL.)

- [ ] **Step 2 : Build + flash + observer les 2 états**

Run : `idf.py build flash monitor`
Aller jusqu'à l'écran verre. Attendu :
- Sans verre : bannière **ambre** « Pose ton verre… ».
- Verre posé (le mock confirme après ~quelques secondes) : bannière **verte** « Verre détecté, c'est parti ! », puis passage à la préparation.

- [ ] **Step 3 : Commit**

```bash
git add components/nectar_ui/src/screens/screen_glass_check.c
git commit -m "feat(ui): feedback verre clair (pose ton verre / verre detecte)"
```

---

## Task 8 : Refonte de l'Accueil (2 grandes tuiles)

**But :** remplacer l'accueil « métriques » par l'accueil festif à 2 tuiles (Le Menu / Composer mon verre) + statut simple + bouton Service discret, conformément à la maquette validée. Seul écran avec changement structurel.

**Files:**
- Modify : `components/nectar_ui/src/screens/screen_home.c`

**Interfaces:**
- Consumes : `app_controller_open_recipes()`, `app_controller_open_custom_mix()`, `app_controller_open_admin_auth()`, `ui_state_model_get()`, styles & couleurs & polices du thème.
- Produces : `screen_home_create(lv_obj_t *)` et `screen_home_refresh(void)` (signatures inchangées).

- [ ] **Step 1 : Réécrire `screen_home.c`**

Remplacer **tout** le contenu de `components/nectar_ui/src/screens/screen_home.c` par :

```c
#include "screen_home.h"

#include "app_controller.h"
#include "recipe_model.h"
#include "ui_metrics.h"
#include "ui_palette.h"
#include "ui_styles.h"
#include "ui_typography.h"
#include "ui_state_model.h"

static lv_obj_t *s_home_status_chip;

static const char *home_status_text(app_machine_state_t machine_state)
{
    switch (machine_state) {
    case APP_MACHINE_ERROR:        return "Indisponible";
    case APP_MACHINE_UNAVAILABLE:  return "Un cocktail en pause";
    case APP_MACHINE_MAINTENANCE:  return "Mode service";
    default:                       return "Prêt à servir";
    }
}

static void home_open_recipes_cb(lv_event_t *event)
{
    (void)event;
    app_controller_open_recipes();
}

static void home_open_mix_cb(lv_event_t *event)
{
    (void)event;
    app_controller_open_custom_mix();
}

static void home_open_admin_cb(lv_event_t *event)
{
    (void)event;
    app_controller_open_admin_auth();
}

static lv_obj_t *home_create_tile(lv_obj_t *parent,
                                  const char *icon,
                                  const char *title_text,
                                  const char *subtitle_text,
                                  bool primary,
                                  lv_event_cb_t event_cb)
{
    lv_obj_t *tile = lv_btn_create(parent);
    lv_obj_t *icon_label = lv_label_create(tile);
    lv_obj_t *title = lv_label_create(tile);
    lv_obj_t *subtitle = lv_label_create(tile);

    lv_obj_remove_style_all(tile);
    lv_obj_add_style(tile, ui_style_card(), 0);
    lv_obj_set_size(tile, 356, 196);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(tile, event_cb, LV_EVENT_CLICKED, NULL);

    if (primary) {
        lv_obj_set_style_bg_color(tile, ui_color_accent(), 0);
        lv_obj_set_style_bg_grad_color(tile, ui_color_accent_secondary(), 0);
        lv_obj_set_style_bg_grad_dir(tile, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_border_width(tile, 0, 0);
        lv_obj_set_style_shadow_color(tile, ui_color_accent_glow(), 0);
        lv_obj_set_style_shadow_opa(tile, LV_OPA_40, 0);
    } else {
        lv_obj_set_style_border_width(tile, 2, 0);
        lv_obj_set_style_border_color(tile, lv_color_hex(0xFFD9B8), 0);
    }

    lv_obj_add_style(icon_label, ui_style_title(), 0);
    lv_obj_set_style_text_font(icon_label, ui_font_hero(), 0);
    lv_obj_set_style_text_color(icon_label, primary ? ui_color_text_ink() : ui_color_accent(), 0);
    lv_label_set_text(icon_label, icon);
    lv_obj_align(icon_label, LV_ALIGN_TOP_LEFT, 20, 18);

    lv_obj_add_style(title, ui_style_title(), 0);
    lv_obj_set_style_text_color(title, primary ? ui_color_text_ink() : ui_color_accent(), 0);
    lv_label_set_text(title, title_text);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 20, 92);

    lv_obj_add_style(subtitle, ui_style_body(), 0);
    lv_obj_set_style_text_color(subtitle, primary ? ui_color_text_ink() : ui_color_text_secondary(), 0);
    lv_obj_set_width(subtitle, 312);
    lv_label_set_long_mode(subtitle, LV_LABEL_LONG_WRAP);
    lv_label_set_text(subtitle, subtitle_text);
    lv_obj_align(subtitle, LV_ALIGN_TOP_LEFT, 20, 130);

    return tile;
}

void screen_home_create(lv_obj_t *screen)
{
    lv_obj_t *eyebrow;
    lv_obj_t *greeting;
    lv_obj_t *tiles;
    lv_obj_t *service_btn;
    lv_obj_t *service_label;

    eyebrow = lv_label_create(screen);
    lv_obj_add_style(eyebrow, ui_style_overline(), 0);
    lv_label_set_text(eyebrow, "NECTAR · LE BAR");
    lv_obj_align(eyebrow, LV_ALIGN_TOP_LEFT, UI_MARGIN_X, UI_MARGIN_TOP);

    greeting = lv_label_create(screen);
    lv_obj_add_style(greeting, ui_style_title(), 0);
    lv_obj_set_style_text_font(greeting, ui_font_display(), 0);
    lv_obj_set_width(greeting, 560);
    lv_label_set_long_mode(greeting, LV_LABEL_LONG_WRAP);
    lv_label_set_text(greeting, "Qu'est-ce qui te ferait plaisir ?");
    lv_obj_align(greeting, LV_ALIGN_TOP_LEFT, UI_MARGIN_X, UI_MARGIN_TOP + 22);

    s_home_status_chip = lv_label_create(screen);
    lv_obj_add_style(s_home_status_chip, ui_style_badge(), 0);
    lv_obj_set_style_bg_color(s_home_status_chip, lv_color_hex(0xE6F6EA), 0);
    lv_obj_set_style_text_color(s_home_status_chip, ui_color_success(), 0);
    lv_obj_align(s_home_status_chip, LV_ALIGN_TOP_RIGHT, -UI_MARGIN_X, UI_MARGIN_TOP);

    tiles = lv_obj_create(screen);
    lv_obj_remove_style_all(tiles);
    lv_obj_set_size(tiles, UI_CONTENT_WIDTH, 196);
    lv_obj_align(tiles, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_set_flex_flow(tiles, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(tiles, 32, 0);
    lv_obj_clear_flag(tiles, LV_OBJ_FLAG_SCROLLABLE);

    home_create_tile(tiles, LV_SYMBOL_LIST, "Le Menu",
                     "Nos cocktails prêts à servir.", true, home_open_recipes_cb);
    home_create_tile(tiles, LV_SYMBOL_EDIT, "Composer mon verre",
                     "Crée ton propre mélange.", false, home_open_mix_cb);

    service_btn = lv_btn_create(screen);
    lv_obj_remove_style_all(service_btn);
    lv_obj_add_style(service_btn, ui_style_button_secondary(), 0);
    lv_obj_add_style(service_btn, ui_style_button_secondary_pressed(), LV_STATE_PRESSED);
    lv_obj_set_size(service_btn, 160, 52);
    lv_obj_align(service_btn, LV_ALIGN_BOTTOM_RIGHT, -UI_MARGIN_X, -UI_MARGIN_BOTTOM);
    lv_obj_add_event_cb(service_btn, home_open_admin_cb, LV_EVENT_CLICKED, NULL);
    service_label = lv_label_create(service_btn);
    lv_label_set_text(service_label, LV_SYMBOL_SETTINGS "  Service");
    lv_obj_center(service_label);

    screen_home_refresh();
}

void screen_home_refresh(void)
{
    const ui_state_model_t *state = ui_state_model_get();

    if (s_home_status_chip == NULL) {
        return;
    }

    lv_label_set_text(s_home_status_chip, home_status_text(state->machine_state));
    if (state->machine_state == APP_MACHINE_ERROR) {
        lv_obj_set_style_bg_color(s_home_status_chip, lv_color_hex(0xFCE4E4), 0);
        lv_obj_set_style_text_color(s_home_status_chip, ui_color_error(), 0);
    } else if (state->machine_state == APP_MACHINE_UNAVAILABLE) {
        lv_obj_set_style_bg_color(s_home_status_chip, lv_color_hex(0xFFF3D6), 0);
        lv_obj_set_style_text_color(s_home_status_chip, lv_color_hex(0x9A6500), 0);
    } else {
        lv_obj_set_style_bg_color(s_home_status_chip, lv_color_hex(0xE6F6EA), 0);
        lv_obj_set_style_text_color(s_home_status_chip, ui_color_success(), 0);
    }
}
```

- [ ] **Step 2 : Build + flash + observer**

Run : `idf.py build flash monitor`
Attendu : accueil avec eyebrow « NECTAR · LE BAR », grand titre festif, pastille de statut verte à droite, **2 grandes tuiles** (Le Menu en orange, Composer mon verre en blanc bordé), bouton « Service » discret en bas à droite. Navigation fonctionnelle vers chaque écran.

- [ ] **Step 3 : Commit**

```bash
git add components/nectar_ui/src/screens/screen_home.c
git commit -m "feat(ui): refonte de l'accueil en deux tuiles festives"
```

---

## Auto-revue (writing-plans)

**Couverture du spec :**
- §3 Tokens couleur → Task 2 ✓
- §4 Typographie + accents FR → Task 3 ✓
- §5 Composants → Tasks 4 & 5 ✓
- §6 Règles de contenu + feedback verre → Tasks 6 & 7 ✓
- §7 Correctif R↔B → Task 1 ✓
- §8 Inventaire écrans → re-skin auto (Tasks 2-4) + FR (Task 6) + accueil (Task 8) ✓
- §9 Anti-flicker → contrainte globale respectée (aucune modif du port LVGL, pas d'animation) ✓
- §13 Critères d'acceptation → couverts (voir ci-dessous)

**Critères d'acceptation ↔ tâches :** (1) couleurs chaudes → T1 ; (2) pas de débordement → tailles fixées T4/T5/T8 + à vérifier au flash de chaque tâche ; (3) FR + accents → T3+T6 ; (4) tokens centralisés → T2/T4 ; (5) pas d'info technique publique → T6 ; (6) feedback verre → T7 ; (7) pas de flicker → contrainte globale ; (8) conforme aux maquettes → T4/T8.

**Cohérence des types :** signature `ui_card_recipe_create` mise à jour en T5 (en-tête + .c + appelant `screen_recipes.c`) de façon cohérente ; nouveaux accesseurs `ui_color_info`/`ui_color_family`/`ui_font_wordmark`/`ui_font_hero` déclarés (T2/T3) avant usage (T4/T5/T8).

**Note débordements (critère 2) :** l'échelle typo agrandie peut révéler des coupes sur certaines cartes/anciens écrans ; à chaque `idf.py flash`, vérifier qu'aucun texte n'est tronqué et ajuster largeurs/`LV_LABEL_LONG_WRAP` au cas par cas dans l'écran concerné (l'ajustement fait partie de la tâche qui touche l'écran).
