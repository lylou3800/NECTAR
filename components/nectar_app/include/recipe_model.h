#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECIPE_MODEL_MAX_INGREDIENTS 4
#define RECIPE_MODEL_MAX_ALLERGENS 3

typedef struct {
    const char *name;
    uint16_t amount_ml;
} recipe_ingredient_t;

typedef struct {
    const char *name;
    const char *tagline;
    const void *visual_src;
    bool available;
    uint8_t stock_percent;
    recipe_ingredient_t ingredients[RECIPE_MODEL_MAX_INGREDIENTS];
    uint8_t ingredient_count;
    const char *allergens[RECIPE_MODEL_MAX_ALLERGENS];
    uint8_t allergen_count;
    uint16_t total_ml;
} recipe_model_t;

size_t recipe_model_count(void);
const recipe_model_t *recipe_model_at(size_t index);

#ifdef __cplusplus
}
#endif