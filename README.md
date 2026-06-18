# NECTAR — Firmware

Firmware du distributeur automatique de boissons **NECTAR** (projet tutoré ISTP).
Écran tactile Waveshare 7" (800×480, GT911) piloté par **ESP32-S3** sous **ESP-IDF** et
interface graphique **LVGL**.

## Architecture

| Composant | Rôle |
|---|---|
| `components/nectar_board` | HAL : écran LCD RGB, tactile GT911, port LVGL (double framebuffer PSRAM, flush synchronisé vsync). |
| `components/nectar_app` | Modèles métier (recettes, boissons, allergènes) + machine à états + services capteurs/pompes (actuellement simulés). |
| `components/nectar_ui` | Interface LVGL : écrans, thème (palette / typographie / styles), composants réutilisables, navigation. |
| `main` | Point d'entrée, initialisation board + UI. |

## Compilation

```bash
idf.py build      # compiler
idf.py flash      # flasher la carte
idf.py monitor    # console série
```

## Documentation

Les specs de conception se trouvent dans `docs/superpowers/specs/`.

## Statut

Refonte de l'interface utilisateur en cours (fondations visuelles, complétion fonctionnelle,
ergonomie, fluidité). Voir les specs pour le détail.
