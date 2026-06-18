#include "recipe_model.h"

#include "allergen_model.h"

static const recipe_model_t s_recipes[] = {
    {
        .name = "Citrus Spritz",
        .tagline = "Fresh, bright, quick serve",
        .visual_src = NULL,
        .available = true,
        .stock_percent = 78,
        .ingredients = {
            {.name = "Citrus soda", .amount_ml = 90},
            {.name = "Dry tonic", .amount_ml = 40},
        },
        .ingredient_count = 2,
        .allergens = {ALLERGEN_CITRUS, ALLERGEN_QUININE},
        .allergen_count = 2,
        .total_ml = 130,
    },
    {
        .name = "Ginger Fizz",
            .tagline = "Soft spice, lively finish",
        .visual_src = NULL,
        .available = true,
        .stock_percent = 61,
        .ingredients = {
            {.name = "Ginger ale", .amount_ml = 80},
            {.name = "Dry tonic", .amount_ml = 40},
        },
        .ingredient_count = 2,
        .allergens = {ALLERGEN_GINGER, ALLERGEN_QUININE},
        .allergen_count = 2,
        .total_ml = 120,
    },
    {
        .name = "Ruby Sunset",
            .tagline = "Premium fruit profile",
        .visual_src = NULL,
        .available = false,
        .stock_percent = 0,
        .ingredients = {
            {.name = "Citrus soda", .amount_ml = 70},
            {.name = "Ginger ale", .amount_ml = 60},
        },
        .ingredient_count = 2,
        .allergens = {ALLERGEN_CITRUS, ALLERGEN_GINGER},
        .allergen_count = 2,
        .total_ml = 130,
    },
    {
        .name = "Night Tonic",
            .tagline = "Dry, crisp, very clean",
        .visual_src = NULL,
        .available = true,
        .stock_percent = 32,
        .ingredients = {
            {.name = "Dry tonic", .amount_ml = 110},
            {.name = "Citrus soda", .amount_ml = 20},
        },
        .ingredient_count = 2,
        .allergens = {ALLERGEN_QUININE, ALLERGEN_CITRUS},
        .allergen_count = 2,
        .total_ml = 130,
    },
};

size_t recipe_model_count(void)
{
    return sizeof(s_recipes) / sizeof(s_recipes[0]);
}

const recipe_model_t *recipe_model_at(size_t index)
{
    if (index >= recipe_model_count()) {
        return NULL;
    }

    return &s_recipes[index];
}