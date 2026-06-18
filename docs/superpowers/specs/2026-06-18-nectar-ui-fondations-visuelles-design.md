# NECTAR — Refonte UI · Sous-projet 1 : Fondations visuelles

**Date :** 2026-06-18
**Statut :** Design validé en brainstorming — en attente de relecture utilisateur
**Périmètre :** Sous-projet 1 d'une refonte UI décomposée en 4 (voir « Place dans la refonte globale »)

---

## 1. Contexte & objectif

NECTAR est un distributeur automatique de boissons (projet tutoré ISTP, écran tactile
Waveshare 7" 800×480, ESP32-S3, LVGL 8.x). Le firmware est proprement structuré :
`nectar_board` (HAL écran/tactile + port LVGL), `nectar_app` (modèles + machine à états,
services capteurs/pompes **simulés**), `nectar_ui` (11 écrans + thème + composants).

L'UI actuelle est fonctionnelle mais souffre de trois problèmes visuels majeurs, constatés
sur photos du matériel réel et par lecture du code :

1. **Rendu couleur inversé (le plus grave).** Le thème codé est *chaud* (or `#D89A42`,
   crème `#F3E5D2`) mais s'affiche en **cyan/bleu froid** à l'écran → inversion des canaux
   **Rouge↔Bleu** (panneau RGB câblé/interprété en BGR).
2. **UI en anglais** alors que le projet et les utilisateurs sont francophones.
3. **Débordements de texte** (cartes recettes coupées, cartes métriques qui se chevauchent)
   et échelle typographique trop petite pour un usage debout sur 7".

**Objectif du sous-projet 1 :** poser des fondations visuelles saines — rendu couleur
correct, design system « Blanc Fruité » festif et professionnel, passage complet en
français (accents inclus), correction des débordements — **sans modifier le modèle
fonctionnel** (recettes/réservoirs traités au sous-projet 2). À la fin, les écrans existants
doivent être beaux, lisibles, en français et rendus dans les bonnes couleurs, en conservant
la fluidité actuelle (zéro flickering/tearing).

---

## 2. Direction visuelle retenue : « Blanc Fruité »

Fond clair, couleurs festives, esprit joyeux de la mascotte Nectar (abricot orange), tout en
restant pro et lisible. Validée en brainstorming (mockups dans `.superpowers/brainstorm/`).

- **Ambiance :** fond blanc chaud, orange de marque, **une couleur de fruit par famille de
  cocktail** (reconnaissance immédiate + festif), statuts colorés clairs.
- **Ton éditorial :** tutoiement chaleureux et festif (« Choisis ton cocktail », « Pose ton
  verre et c'est parti »).

---

## 3. Tokens de couleur

Les couleurs restent centralisées dans `components/nectar_ui/src/theme/ui_palette.c`
(contrat de fonctions `ui_color_*()` conservé, valeurs passées en thème clair + ajout de
nouveaux accesseurs). **Aucune couleur en dur dans les écrans.**

### Neutres & texte
| Token | Hex | Usage |
|---|---|---|
| `background` | `#FFFCF8` | fond d'écran (blanc chaud ; basculable en `#FFFFFF` pur) |
| `surface` | `#FFFFFF` | cartes |
| `surface_alt` | `#FFF7F0` | inserts, surfaces secondaires |
| `line` | `#F0E6DC` | bordures/séparateurs |
| `text_primary` | `#2B1B12` | titres, valeurs |
| `text_secondary` | `#6B5648` | corps |
| `text_muted` | `#A2917F` | légendes, labels |

### Marque (orange Nectar)
| Token | Hex | Usage |
|---|---|---|
| `accent` | `#FF8A3D` | couleur principale |
| `accent_grad` | `#FFB23D` | fin du dégradé bouton |
| `accent_deep` | `#E0631A` | texte/icône orange sur fond clair (contraste AA) |
| `accent_soft` | `#FFF1E6` | fonds de boutons secondaires / pastilles |

### Couleurs par famille de cocktail (festif + reconnaissance)
| Famille | Hex |
|---|---|
| Agrume | `#FF8A3D` |
| Fruit rouge | `#F0356B` |
| Gingembre / herbe | `#7BC043` |
| Tonic / botanique | `#9B5CD6` |
| Tropical | `#16C2B3` |
| Citron / miel | `#FFC93C` |

> Note SP1 : chaque recette se voit attribuer une famille/couleur. Au SP1 on câble les
> 4 recettes existantes ; l'extension au catalogue complet relève du SP2.

### États
| Token | Hex | Usage |
|---|---|---|
| `success` | `#2E9E4F` | OK, prêt, niveau sain |
| `warning` | `#F5A623` | niveau bas, attention |
| `error` | `#E5484D` | épuisé, erreur, sécurité |
| `info` | `#3D7BFF` | information neutre |

### Contraste
Tous les couples texte/fond visent au minimum WCAG AA (texte foncé sur clair ; texte blanc
sur orange uniquement en gras ≥ 17 px ou sur `accent_deep`).

---

## 4. Typographie

LVGL n'embarque actuellement que **Montserrat 14/16/20/24, une seule graisse**, et **sans
glyphes accentués**. Deux conséquences :

1. **Génération de polices obligatoire** (via `lv_font_conv` / convertisseur LVGL) couvrant
   les accents français. Plage de glyphes : ASCII `0x20–0x7F` + Latin-1 `0xA0–0xFF`
   (à â ä ç è é ê ë î ï ô ö ù û ü, majuscules accentuées, « » ° …) + `Œ/œ` (`0x152/0x153`),
   `’` (`0x2019`), `…` (`0x2026`).
2. **Deux familles** (choix B du brainstorming) :
   - **Montserrat** (Regular 400 + ExtraBold 800, et SemiBold 600 pour les sections) pour
     toute l'UI/lisibilité.
   - **Police display arrondie festive** (type *Baloo 2* ExtraBold) réservée au **wordmark
     « NECTAR » et aux grands titres d'écran** (apporte le caractère festif/mascotte).

### Échelle (agrandie pour usage debout sur 7")
| Rôle | Taille px | Graisse | Famille |
|---|---|---|---|
| eyebrow / overline | 12 | 800, +3 tracking, MAJ | Montserrat |
| légende | 14 | 400 | Montserrat |
| corps | 17 | 400 | Montserrat |
| section / nom de carte | 22 | 600 | Montserrat |
| titre d'écran | ~28–31 | 800 | **Display (Baloo)** |
| nombre/héros (ex. « 84 % ») | ~40–46 | 800 | Montserrat |
| libellé bouton | 17–18 | 800 | Montserrat |
| wordmark NECTAR | ~22–24 | 800 | **Display (Baloo)** |

> **Budget flash :** chaque taille×graisse est une fonte compilée. On limite le nombre de
> tailles réellement générées (≈ 6–8) et la plage de glyphes pour rester raisonnable. Liste
> exacte figée en phase de plan d'implémentation.

`ui_typography.c` exposera les accesseurs (`ui_font_display()`, `ui_font_title()`,
`ui_font_heading()`, `ui_font_body()`, `ui_font_caption()`, `ui_font_hero()`, etc.) pointant
vers les nouvelles fontes.

---

## 5. Composants (design system)

Restylés dans `components/nectar_ui/src/components/` et `theme/ui_styles.c`. Référence
visuelle : mockup `composants.html`.

- **En-tête d'écran (chrome) :** bouton retour (icône, cible ≥ 44 px), eyebrow + titre
  (display), pastille de statut à droite. Helper commun (`ui_chrome`).
- **Boutons :** principal (orange dégradé, texte blanc, coins ~26 px, ombre douce),
  secondaire (fond `accent_soft`, texte `accent_deep`, bordure), fantôme (bordure neutre),
  désactivé (gris chaud, sans ombre). États *pressed* conservés (transitions courtes).
- **Carte cocktail :** capsule colorée (dégradé de la famille) + verre stylisé, nom (22/800),
  tagline (14), pastille de famille + pastille de statut, mini-badge allergène. Tailles fixées
  pour **éliminer tout débordement** (texte en `LV_LABEL_LONG_DOT`/wrap maîtrisé).
- **Carte info / métrique :** label (overline) + grande valeur.
- **Jauge de réservoir :** barre arrondie, remplissage dégradé piloté par seuils
  (vert ≥ seuil OK, ambre→rouge si bas), `% + litres` à droite. *(Affichée surtout côté
  Service.)*
- **Bannières d'état :** succès / alerte / sécurité (fond teinté + icône + texte).
- **Indicateur de détection de verre :** voir §6.
- **Pastilles (tags) :** statut et famille, coins pleins, lisibles.

---

## 6. Règles de contenu & feedback utilisateur

**Principe directeur :** le flux public reste simple et festif ; le détail technique vit
uniquement côté **Service/Admin**.

### À retirer des écrans utilisateurs
- Mentions « ±5 mL », « dosage précis », précision technique.
- « Détection de verre active/automatique » présentée comme une fonctionnalité/statut.
- Noms d'étapes internes (« Dose check », « 2 lines », etc.).
- « Stock coverage % », couverture de stock, télémétrie.

### À conserver (utile à l'utilisateur)
- Nom + tagline du cocktail, **volume du verre (mL)**, ingrédients (sur le détail),
  allergènes, statut simple (**Prêt / Indisponible**).
- Sur le **mix perso** : les volumes que l'utilisateur choisit restent affichés (c'est son
  action), mais sans discours marketing sur la précision.

### Détection de verre — feedback (nouveau, demandé)
La détection est **silencieuse quand tout va bien**. Elle ne se manifeste que pour guider :
- **Verre absent au démarrage du service** → écran/prompt clair et amical :
  « **Pose ton verre 🥃** » + visuel de verre en attente (pulsation légère = SP4 ; visuel
  statique défini ici). Aucune distribution tant que le verre n'est pas détecté.
- **Verre détecté** → confirmation brève (« C'est parti ! ») puis passage à la préparation.
- **Verre retiré en cours de service** → bannière **sécurité** plein cadre :
  « **Remets ton verre** — service en pause », visuel `error/warning`.

### Étape « préparation »
Libellés amicaux (« On prépare ton verre… ✨ ») + barre de progression, sans noms d'étapes
techniques.

### Côté Service/Admin (technicien)
Le détail technique est **légitime et conservé** : niveaux réservoirs (% + L), statuts,
recalibrage capteurs, journal d'alertes (style maquette de référence « ADMINISTRATION :
NIVEAUX DES RÉSERVOIRS »). L'enrichissement complet (8 réservoirs) relève du SP2 ; au SP1 on
restyle l'écran maintenance existant.

---

## 7. Correction du rendu couleur (R↔B) — le correctif n°1

**Hypothèse :** le panneau RGB est câblé/interprété en **BGR** : les groupes de lignes de
données R et B sont permutés par rapport au RGB565 attendu par LVGL/esp_lcd. Symptôme
cohérent sur toutes les photos (or→bleu, crème→bleu pâle, vert peu affecté).

**Démarche :**
1. **Diagnostic sur matériel** avec une mire de couleurs pures (rouge / vert / bleu pleins).
   Si « rouge codé » s'affiche bleu et inversement → confirmation R↔B.
2. **Correctif privilégié (coût nul à l'exécution) :** permuter les groupes GPIO **R0–R4** et
   **B0–B4** dans `data_gpio_nums` (`board_display.c`, `board_panel_init`). Le vert (G0–G5)
   reste inchangé.
3. **Alternatives de repli** si le diagnostic diffère : drapeau d'ordre de couleur du panneau
   si disponible, ou conversion logicielle en dernier recours (déconseillé : coût par pixel
   avec framebuffer direct + full_refresh).

**Critère :** après correctif, une mire rouge est rouge, l'orange `#FF8A3D` est orange.
Le test se fait *avant* d'appliquer le nouveau thème (sinon le thème clair masque le blanc
mais pas les accents).

---

## 8. Inventaire des écrans (ce que change le SP1)

| Écran (fichier) | Changements SP1 |
|---|---|
| `screen_boot` | Splash : wordmark NECTAR (display) + mascotte, fond clair, animation conservée. |
| `screen_home` | Refonte « Accueil » : salutation festive + mascotte, statut simple, **2 grandes tuiles** (Le Menu / Composer mon verre), bouton discret « Service ». Retrait des infos techniques. |
| `screen_recipes` | Restyle « Menu » en cartes cocktail color-codées (présentation type carrousel). Correction des débordements. *(Logique de swipe = SP3.)* |
| `screen_drink_detail` | Détail utile : nom, tagline, volume, ingrédients, allergènes, statut. Retrait stock %/étapes. |
| `screen_custom_mix` | Restyle des sélecteurs de volume ; retrait du discours « ±5 mL ». *(Passage à N ingrédients/8 réservoirs = SP2.)* |
| `screen_glass_check` | **Feedback « Pose ton verre »** (§6), visuel amical clair. |
| `screen_preparing` | Libellés amicaux + barre de progression, sans étapes techniques. |
| `screen_ready` | « C'est prêt ! » festif + invite à retirer le verre. |
| `screen_error` | Bannière `error`/`warning` claire, ton humain ; cas « verre retiré » (§6). |
| `screen_admin_auth` | Restyle pavé PIN (cibles larges), en français. |
| `screen_maintenance` | Restyle « Service » : conserve le détail technique (niveaux, statuts). Enrichissement 8 réservoirs = SP2. |

Tous les libellés passent en **français** (chaînes centralisées si simple, sinon par écran).

---

## 9. Performance & anti-flicker (exigence forte)

Le port LVGL actuel (`lvgl_port.c`) fait déjà du **double framebuffer en PSRAM, flush
synchronisé au vsync, `direct_mode` + `full_refresh`** → pas de tearing. **À préserver
intégralement.** Contraintes de design pour ne rien dégrader :
- Pas de redraw inutile : `ui_state_model_request_refresh()` reste piloté par changement
  d'état (déjà le cas) ; les écrans ne se reconstruisent que sur changement de vue.
- Dégradés/ombres : usage raisonné (le panneau a un budget de fill-rate à 18 MHz pclk).
  Préférer des dégradés simples ; éviter les ombres très larges multipliées.
- Le thème clair n'a aucun surcoût de bande passante (un pixel blanc coûte comme un autre).
- Les **animations** (transitions d'écran, pulsation du prompt verre) sont **hors SP1**
  (sous-projet 4 Fluidité) — mais le SP1 ne doit rien introduire qui provoque flicker.

---

## 10. Place dans la refonte globale

| # | Sous-projet | Statut |
|---|---|---|
| **1** | **Fondations visuelles** (ce doc) | en cours |
| 2 | Complétion fonctionnelle CDC (8 réservoirs, vraies recettes, mix perso complet, admin enrichi) | à venir |
| 3 | Ergonomie & navigation (carrousel interactif, chrome cohérent, feedback tactile) | à venir |
| 4 | Fluidité & perf (transitions, animations, cache d'écrans) | à venir |

---

## 11. Hors-périmètre (SP1)

- Modèle de données 8 réservoirs / vraies recettes / mix à N ingrédients → **SP2**.
- Interaction de swipe du carrousel, refonte de la navigation → **SP3**.
- Transitions d'écran et animations → **SP4**.
- Câblage capteurs/pompes réels (services restent simulés).
- Photos réelles de cocktails (asset pipeline) : non retenu au SP1 (verres stylisés en CSS/
  vecteur) ; à rediscuter si budget flash le permet.

---

## 12. Risques & dépendances

- **Diagnostic couleur** : le correctif exact dépend d'une vérification matérielle ; plan B
  prévu (§7).
- **Budget flash des polices** : maîtrisé en limitant tailles et plage de glyphes.
- **Écran physique clair** : peut révéler poussières/reflets (cosmétique, accepté).
- Dépend de l'accès au matériel pour valider rendu et fluidité.

---

## 13. Critères d'acceptation

1. Sur matériel, les couleurs chaudes s'affichent chaudes (mire rouge = rouge ; orange de
   marque = orange). Plus aucune dérive cyan.
2. Aucun texte coupé/chevauché sur les 11 écrans en 800×480.
3. 100 % des chaînes visibles sont en français, **accents rendus correctement**.
4. Tokens couleur/typo centralisés ; aucune couleur en dur dans les écrans.
5. Les écrans publics ne contiennent aucune info technique interne (±5 mL, % stock, étapes
   internes, « détection active ») ; le détail technique subsiste côté Service.
6. Verre absent → prompt « Pose ton verre » clair ; verre retiré → feedback sécurité.
7. Aucune régression de fluidité : pas de flicker/tearing (double-buffer vsync préservé).
8. Le design correspond aux mockups « Blanc Fruité » validés.
